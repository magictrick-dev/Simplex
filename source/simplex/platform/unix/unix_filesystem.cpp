#include <simplex/platform/unix/unix_filesystem.hpp>
#if defined(__linux__)

#include <simplex/dynamic_string.hpp>
#include <simplex/hashed_sparse_map.hpp>

#include <utils/logging.hpp>
#include <utils/system/memory_alloc.hpp>

#include <mutex>
#include <shared_mutex>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>

// NOTE(Chris): The registry associates every open descriptor with its book-keeping
//              record (tracked pointer, flags, owning path). It exists purely for
//              registration/unregistration in open_file/close_file and for a future
//              front-end API that enumerates active handles. It is NOT consulted on the
//              read/write/seek hot path: those receive the record pointer as the public
//              file_handle_t and operate on it directly. Because hashed_sparse_map nodes
//              never move (even on rehash), the record pointer stays valid for the life of
//              the handle. The mutex only guards the map structure during open/close.
// TODO(Chris): Add a front-end API to access all active file handles.
static std::shared_mutex handles_mutex;
static spx::hashed_sparse_map<int, spx::fs::unix_file_handle> handles;

// --- Internal helpers --------------------------------------------------------

// NOTE(Chris): read()/write() cap a single transfer at a platform-defined maximum
//              (0x7ffff000 on Linux), so positional I/O is chunked through this bound.
static constexpr size_t unix_max_io_chunk = 0x7FFFF000;

// NOTE(Chris): Newly created files get the conventional default mode; the umask
//              of the calling process narrows it as usual.
static constexpr mode_t unix_default_create_mode = 0644;

// NOTE(Chris): String views are not guaranteed to be null-terminated, so we
//              materialize a null-terminated copy whenever we hand a path off to
//              the POSIX APIs.
static inline spx::dynamic_string<char>
make_null_terminated(spx::string_view<char> path)
{
    return spx::dynamic_string<char>(path.data(), path.size());
}

static inline int
translate_open_flag(PlatformFilesystemOpenFlag open_flag)
{
    switch (open_flag)
    {
        case PlatformFilesystemOpenFlag_Read:       return O_RDONLY;
        case PlatformFilesystemOpenFlag_Write:      return O_WRONLY;
        case PlatformFilesystemOpenFlag_ReadWrite:  return O_RDWR;
    }
    SIMPLEX_NO_REACH("Unhandled open flag.");
    return 0;
}

static inline int
translate_creation_flag(PlatformFilesystemCreationFlag creation_flag)
{
    switch (creation_flag)
    {
        case PlatformFilesystemCreationFlag_CreateAlways:   return O_CREAT | O_TRUNC;
        case PlatformFilesystemCreationFlag_CreateNew:      return O_CREAT | O_EXCL;
        case PlatformFilesystemCreationFlag_OpenAlways:     return O_CREAT;
        case PlatformFilesystemCreationFlag_OpenOnly:       return 0;
        case PlatformFilesystemCreationFlag_TruncateOnly:   return O_TRUNC;
    }
    SIMPLEX_NO_REACH("Unhandled creation flag.");
    return 0;
}

// NOTE(Chris): POSIX has no mandatory open-time sharing mode equivalent to Win32's
//              dwShareMode; access arbitration is advisory (fcntl/flock) and not
//              enforced here. The share flag is recorded on the handle for parity and
//              future use, but does not influence the open call.
static inline void
acknowledge_share_flag(PlatformFilesystemShareFlag share_flag)
{
    switch (share_flag)
    {
        case PlatformFilesystemShareFlag_ShareRead:
        case PlatformFilesystemShareFlag_ShareWrite:
        case PlatformFilesystemShareFlag_ShareReadWrite:    return;
    }
    SIMPLEX_NO_REACH("Unhandled share flag.");
}

// NOTE(Chris): Folds an errno value into our platform-agnostic result enum so that
//              callers never have to reason about POSIX specifics.
static inline PlatformFilesystemResult
translate_errno(int error_code)
{
    switch (error_code)
    {
        case ENOENT:
        case ENOTDIR:
        case ENAMETOOLONG:
        case EINVAL:        return PlatformFilesystemResult_PathInvalid;
        case EAGAIN:
        case ETXTBSY:       return PlatformFilesystemResult_PathUnavailable;
        case EACCES:
        case EPERM:
        case EROFS:         return PlatformFilesystemResult_PathAccessInvalid;
        default:            return PlatformFilesystemResult_PathInvalid;
    }
}

// NOTE(Chris): The public file_handle_t is simply the address of the book-keeping
//              record, so resolving it is a cast - no map lookup, no lock.
static inline spx::fs::unix_file_handle*
as_internal_handle(file_handle_t handle)
{
    return (spx::fs::unix_file_handle*)handle;
}

static inline bool
query_native_size(int native_handle, uint64_t *out_size)
{
    struct stat status = {};
    if (fstat(native_handle, &status) != 0) return false;
    *out_size = (uint64_t)status.st_size;
    return true;
}

// NOTE(Chris): readlink on /proc/self/exe yields the absolute, symlink-resolved path
//              of the running binary. It does not null-terminate and silently truncates
//              when the buffer is too small, so we grow until the result fits.
static spx::dynamic_string<char>
query_executable_path()
{
    size_t capacity = PATH_MAX;
    while (true)
    {

        char *raw = (char*)simplex_memory_alloc(capacity);
        ssize_t length = readlink("/proc/self/exe", raw, capacity);

        if (length < 0)
        {
            simplex_memory_free(raw);
            return spx::dynamic_string<char>();
        }

        // A result strictly less than the capacity means the path fit (readlink does
        // not null-terminate, so an exactly-full buffer is ambiguous and we grow).
        if ((size_t)length < capacity)
        {
            spx::dynamic_string<char> result(raw, (size_t)length);
            simplex_memory_free(raw);
            return result;
        }

        simplex_memory_free(raw);
        capacity *= 2;

    }
}

namespace spx::fs
{

    spx::string_view<char>
    get_executable_directory()
    {

        // The executable directory is immutable, so we resolve it exactly once.
        static spx::dynamic_string<char> cached_directory;
        static std::once_flag init_flag;

        std::call_once(init_flag, [](){

            spx::dynamic_string<char> full_path = query_executable_path();

            size_t last_separator = (size_t)-1;
            for (size_t i = 0; i < full_path.size(); ++i)
            {
                if (full_path[i] == '/') last_separator = i;
            }

            if (last_separator != (size_t)-1)
                cached_directory = spx::dynamic_string<char>(full_path.data(), last_separator);
            else
                cached_directory = full_path;

            spx::logger::dispatch_diagnostic_log("Resolved executable directory: {}.", cached_directory.c_str());

        });

        return spx::string_view<char>(cached_directory.c_str(), cached_directory.size());

    }

    spx::string_view<char>
    get_working_directory()
    {

        // The working directory is mutable at runtime, so we re-query each call and
        // cache the result per-thread to keep the returned view's storage stable.
        thread_local spx::dynamic_string<char> cached_working_directory;

        size_t capacity = PATH_MAX;
        while (true)
        {

            char *raw = (char*)simplex_memory_alloc(capacity);
            if (getcwd(raw, capacity) != NULL)
            {
                cached_working_directory = spx::dynamic_string<char>(raw);
                simplex_memory_free(raw);
                break;
            }

            // ERANGE means the buffer was too small; grow and retry. Anything else
            // is a hard failure, so we surface an empty view.
            int error_code = errno;
            simplex_memory_free(raw);
            if (error_code != ERANGE)
            {
                cached_working_directory.clear();
                break;
            }
            capacity *= 2;

        }

        return spx::string_view<char>(cached_working_directory.c_str(), cached_working_directory.size());

    }

    PlatformFilesystemResult
    path_is_file(spx::string_view<char> path)
    {

        spx::dynamic_string<char> native_path = make_null_terminated(path);

        struct stat status = {};
        if (stat(native_path.c_str(), &status) != 0) return PlatformFilesystemResult_PathInvalid;
        if (S_ISDIR(status.st_mode)) return PlatformFilesystemResult_PathNotFile;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    path_is_directory(spx::string_view<char> path)
    {

        spx::dynamic_string<char> native_path = make_null_terminated(path);

        struct stat status = {};
        if (stat(native_path.c_str(), &status) != 0) return PlatformFilesystemResult_PathInvalid;
        if (S_ISDIR(status.st_mode)) return PlatformFilesystemResult_OK;
        return PlatformFilesystemResult_PathNotDirectory;

    }

    PlatformFilesystemResult
    path_is_valid(spx::string_view<char> path)
    {

        spx::dynamic_string<char> native_path = make_null_terminated(path);

        struct stat status = {};
        if (stat(native_path.c_str(), &status) != 0) return PlatformFilesystemResult_PathInvalid;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    file_is_ready(spx::string_view<char> path)
    {

        spx::dynamic_string<char> native_path = make_null_terminated(path);

        struct stat status = {};
        if (stat(native_path.c_str(), &status) != 0) return PlatformFilesystemResult_PathInvalid;
        if (S_ISDIR(status.st_mode)) return PlatformFilesystemResult_PathNotFile;

        // A successful open is the closest signal that the file can be used.
        int probe = open(native_path.c_str(), O_RDONLY);
        if (probe < 0)
        {
            int error_code = errno;
            if (error_code == ETXTBSY || error_code == EAGAIN)
                return PlatformFilesystemResult_PathUnavailable;
            return PlatformFilesystemResult_FileNotReady;
        }

        close(probe);
        return PlatformFilesystemResult_OK;

    }

    spx::dynamic_string<char>
    canonicalize_path(spx::string_view<char> path)
    {

        spx::dynamic_string<char> native_path = make_null_terminated(path);

        // realpath resolves symlinks and relative segments, but only for paths that
        // exist. The contract does not require existence, so we fall back to making
        // the path absolute against the working directory when resolution fails.
        char resolved[PATH_MAX];
        if (realpath(native_path.c_str(), resolved) != NULL)
            return spx::dynamic_string<char>(resolved);

        if (!native_path.empty() && native_path[0] == '/')
            return native_path;

        spx::string_view<char> working = get_working_directory();
        spx::dynamic_string<char> result(working.data(), working.size());
        result += '/';
        result += native_path;
        return result;

    }

    PlatformFilesystemResult
    get_file_size(spx::string_view<char> path, size_t *file_size)
    {

        SIMPLEX_CHECK_PTR(file_size);

        spx::dynamic_string<char> native_path = make_null_terminated(path);

        struct stat status = {};
        if (stat(native_path.c_str(), &status) != 0)
            return translate_errno(errno);

        if (S_ISDIR(status.st_mode))
            return PlatformFilesystemResult_PathNotFile;

        *file_size = (size_t)status.st_size;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    read_entire_file(spx::string_view<char> path,
                     void *buffer,
                     size_t buffer_size,
                     size_t *bytes_read)
    {

        SIMPLEX_CHECK_PTR(buffer);
        SIMPLEX_CHECK_PTR(bytes_read);
        *bytes_read = 0;

        spx::dynamic_string<char> native_path = make_null_terminated(path);

        int native_handle = open(native_path.c_str(), O_RDONLY);
        if (native_handle < 0)
            return translate_errno(errno);

        uint64_t file_size = 0;
        if (!query_native_size(native_handle, &file_size))
        {
            close(native_handle);
            return PlatformFilesystemResult_FileNotReady;
        }

        if (file_size > buffer_size)
        {
            close(native_handle);
            return PlatformFilesystemResult_InsufficientBufferSize;
        }

        uint64_t total_read = 0;
        uint8_t *cursor = (uint8_t*)buffer;
        while (total_read < file_size)
        {
            uint64_t remaining = file_size - total_read;
            size_t to_read = (remaining > unix_max_io_chunk) ? unix_max_io_chunk : (size_t)remaining;
            ssize_t chunk_read = pread(native_handle, cursor + total_read, to_read, (off_t)total_read);
            if (chunk_read < 0)
            {
                if (errno == EINTR) continue; // Interrupted before any transfer; retry.
                close(native_handle);
                return PlatformFilesystemResult_FileNotReady;
            }
            if (chunk_read == 0) break; // Reached EOF earlier than reported size.
            total_read += (uint64_t)chunk_read;
        }

        close(native_handle);
        *bytes_read = (size_t)total_read;

        return (total_read == file_size)
            ? PlatformFilesystemResult_OK
            : PlatformFilesystemResult_EOF;

    }

    PlatformFilesystemResult
    write_entire_file(spx::string_view<char> path,
                      void *buffer,
                      size_t buffer_size,
                      size_t *bytes_written)
    {

        SIMPLEX_CHECK_PTR(buffer);
        SIMPLEX_CHECK_PTR(bytes_written);
        *bytes_written = 0;

        spx::dynamic_string<char> native_path = make_null_terminated(path);

        int native_handle = open(native_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, unix_default_create_mode);
        if (native_handle < 0)
            return translate_errno(errno);

        uint64_t total_written = 0;
        const uint8_t *cursor = (const uint8_t*)buffer;
        while (total_written < buffer_size)
        {
            uint64_t remaining = buffer_size - total_written;
            size_t to_write = (remaining > unix_max_io_chunk) ? unix_max_io_chunk : (size_t)remaining;
            ssize_t chunk_written = pwrite(native_handle, cursor + total_written, to_write, (off_t)total_written);
            if (chunk_written < 0)
            {
                if (errno == EINTR) continue; // Interrupted before any transfer; retry.
                close(native_handle);
                *bytes_written = (size_t)total_written;
                return PlatformFilesystemResult_FileNotReady;
            }
            total_written += (uint64_t)chunk_written;
        }

        close(native_handle);
        *bytes_written = (size_t)total_written;

        return PlatformFilesystemResult_OK;

    }


    PlatformFilesystemResult
    open_file(file_handle_t *handle,
              spx::string_view<char> file_path,
              PlatformFilesystemOpenFlag open_flag,
              PlatformFilesystemShareFlag share_flag,
              PlatformFilesystemCreationFlag creation_flag)
    {

        SIMPLEX_CHECK_PTR(handle);
        *handle = NULL;

        spx::dynamic_string<char> native_path = make_null_terminated(file_path);

        int access_flags   = translate_open_flag(open_flag);
        int creation_flags = translate_creation_flag(creation_flag);
        acknowledge_share_flag(share_flag);

        int native_handle = open(native_path.c_str(), access_flags | creation_flags, unix_default_create_mode);
        if (native_handle < 0)
        {
            int error_code = errno;
            spx::logger::dispatch_warning_log("Failed to open file {} (errno {}).", native_path.c_str(), error_code);
            return translate_errno(error_code);
        }

        unix_file_handle record    = {};
        record.handle              = native_handle;
        record.pointer             = 0;
        record.creation_flag       = creation_flag;
        record.open_flag           = open_flag;
        record.share_flag          = share_flag;
        record.path                = std::move(native_path);

        // Cache the size once so seek-bound validation on read handles needs no syscall.
        uint64_t file_size = 0;
        if (query_native_size(native_handle, &file_size)) record.size = (size_t)file_size;

        // Register the record. The map node is address-stable, so its address is a stable
        // public handle the caller carries for the lifetime of the file.
        std::unique_lock<std::shared_mutex> lock(handles_mutex);
        unix_file_handle &stored = handles.insert(native_handle, std::move(record));

        *handle = (file_handle_t)&stored;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    close_file(file_handle_t *handle)
    {

        SIMPLEX_CHECK_PTR(handle);
        if (*handle == NULL) return PlatformFilesystemResult_InvalidHandle;

        unix_file_handle *record = as_internal_handle(*handle);
        int native_handle = record->handle;

        std::unique_lock<std::shared_mutex> lock(handles_mutex);

        // Guard against double-close: only act if the handle is still registered.
        if (handles.find(native_handle) == NULL) return PlatformFilesystemResult_InvalidHandle;

        close(native_handle);
        handles.remove(native_handle); // Frees the node; record must not be touched afterward.

        *handle = NULL;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    read_file(file_handle_t handle, void *buffer, size_t read_size, size_t *amount_read)
    {

        SIMPLEX_CHECK_PTR(buffer);
        SIMPLEX_CHECK_PTR(amount_read);
        *amount_read = 0;

        unix_file_handle *record = as_internal_handle(handle);
        if (record == NULL) return PlatformFilesystemResult_InvalidHandle;

        int native_handle    = record->handle;
        uint64_t offset      = record->pointer;
        uint64_t total_read  = 0;
        uint8_t *cursor      = (uint8_t*)buffer;

        // pread carries the offset per call, so each read is a single syscall and never
        // touches the descriptor's own seek offset - our tracked pointer is authoritative.
        while (total_read < read_size)
        {
            uint64_t position = offset + total_read;
            uint64_t remaining = read_size - total_read;
            size_t to_read = (remaining > unix_max_io_chunk) ? unix_max_io_chunk : (size_t)remaining;

            ssize_t chunk_read = pread(native_handle, cursor + total_read, to_read, (off_t)position);
            if (chunk_read < 0)
            {
                if (errno == EINTR) continue; // Interrupted before any transfer; retry.
                return PlatformFilesystemResult_PathUnavailable;
            }
            if (chunk_read == 0) break; // EOF reached.
            total_read += (uint64_t)chunk_read;
        }

        record->pointer = (size_t)(offset + total_read);
        *amount_read = (size_t)total_read;

        return (total_read == read_size)
            ? PlatformFilesystemResult_OK
            : PlatformFilesystemResult_EOF;

    }

    PlatformFilesystemResult
    write_file(file_handle_t handle, void *buffer, size_t write_size, size_t *amount_written)
    {

        SIMPLEX_CHECK_PTR(buffer);
        SIMPLEX_CHECK_PTR(amount_written);
        *amount_written = 0;

        unix_file_handle *record = as_internal_handle(handle);
        if (record == NULL) return PlatformFilesystemResult_InvalidHandle;

        int native_handle       = record->handle;
        uint64_t offset         = record->pointer;
        uint64_t total_written  = 0;
        const uint8_t *cursor   = (const uint8_t*)buffer;

        while (total_written < write_size)
        {
            uint64_t position = offset + total_written;
            uint64_t remaining = write_size - total_written;
            size_t to_write = (remaining > unix_max_io_chunk) ? unix_max_io_chunk : (size_t)remaining;

            ssize_t chunk_written = pwrite(native_handle, cursor + total_written, to_write, (off_t)position);
            if (chunk_written < 0)
            {
                if (errno == EINTR) continue; // Interrupted before any transfer; retry.
                record->pointer = (size_t)(offset + total_written);
                if (record->pointer > record->size) record->size = record->pointer;
                *amount_written = (size_t)total_written;
                return PlatformFilesystemResult_PathUnavailable;
            }
            total_written += (uint64_t)chunk_written;
        }

        record->pointer = (size_t)(offset + total_written);
        if (record->pointer > record->size) record->size = record->pointer; // Track growth.
        *amount_written = (size_t)total_written;

        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    move_file_pointer(file_handle_t handle, int64_t relative)
    {

        unix_file_handle *record = as_internal_handle(handle);
        if (record == NULL) return PlatformFilesystemResult_InvalidHandle;

        int64_t target = (int64_t)record->pointer + relative;
        if (target < 0) return PlatformFilesystemResult_EOF; // Moved before the start.

        // A read-only handle may not seek past EOF; writing handles zero-pad on write.
        // The cached size avoids a per-seek size syscall on the streaming read path.
        if (record->open_flag == PlatformFilesystemOpenFlag_Read && (uint64_t)target > record->size)
            return PlatformFilesystemResult_EOF;

        record->pointer = (size_t)target;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    set_file_pointer(file_handle_t handle, uint64_t absolute)
    {

        unix_file_handle *record = as_internal_handle(handle);
        if (record == NULL) return PlatformFilesystemResult_InvalidHandle;

        if (record->open_flag == PlatformFilesystemOpenFlag_Read && absolute > record->size)
            return PlatformFilesystemResult_EOF;

        record->pointer = (size_t)absolute;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    set_file_pointer_at_bof(file_handle_t handle)
    {

        unix_file_handle *record = as_internal_handle(handle);
        if (record == NULL) return PlatformFilesystemResult_InvalidHandle;

        record->pointer = 0;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    set_file_pointer_at_eof(file_handle_t handle)
    {

        unix_file_handle *record = as_internal_handle(handle);
        if (record == NULL) return PlatformFilesystemResult_InvalidHandle;

        // Re-query live: an explicit seek-to-end must reflect any growth since open, and
        // it is not on a tight loop. Refresh the cache while we are here.
        uint64_t file_size = 0;
        if (!query_native_size(record->handle, &file_size))
            return PlatformFilesystemResult_PathUnavailable;

        record->size = (size_t)file_size;
        record->pointer = (size_t)file_size;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    get_file_pointer_position(file_handle_t handle, size_t *location)
    {

        SIMPLEX_CHECK_PTR(location);

        unix_file_handle *record = as_internal_handle(handle);
        if (record == NULL) return PlatformFilesystemResult_InvalidHandle;

        *location = record->pointer;
        return PlatformFilesystemResult_OK;

    }

}

#endif
