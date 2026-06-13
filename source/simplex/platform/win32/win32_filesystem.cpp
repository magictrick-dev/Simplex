#include <simplex/platform/win32/win32_filesystem.hpp>
#include <simplex/dynamic_string.hpp>
#include <simplex/hashed_sparse_map.hpp>

#include <utils/logging.hpp>
#include <utils/system/memory_alloc.hpp>

#include <mutex>
#include <shared_mutex>

// NOTE(Chris): The registry associates every open native handle with its book-keeping
//              record (tracked pointer, flags, owning path). It exists purely for
//              registration/unregistration in open_file/close_file and for a future
//              front-end API that enumerates active handles. It is NOT consulted on the
//              read/write/seek hot path: those receive the record pointer as the public
//              file_handle_t and operate on it directly. Because hashed_sparse_map nodes
//              never move (even on rehash), the record pointer stays valid for the life of
//              the handle. The mutex only guards the map structure during open/close.
// TODO(Chris): Add a front-end API to access all active file handles.
static std::shared_mutex handles_mutex;
static spx::hashed_sparse_map<HANDLE, spx::fs::win32_file_handle> handles;

// --- Internal helpers --------------------------------------------------------

// NOTE(Chris): String views are not guaranteed to be null-terminated, so we
//              materialize a null-terminated copy whenever we hand a path off to
//              the Win32 ANSI APIs.
static inline spx::dynamic_string<char>
make_null_terminated(spx::string_view<char> path)
{
    return spx::dynamic_string<char>(path.data(), path.size());
}

static inline DWORD
translate_open_flag(PlatformFilesystemOpenFlag open_flag)
{
    switch (open_flag)
    {
        case PlatformFilesystemOpenFlag_Read:       return GENERIC_READ;
        case PlatformFilesystemOpenFlag_Write:      return GENERIC_WRITE;
        case PlatformFilesystemOpenFlag_ReadWrite:  return GENERIC_READ | GENERIC_WRITE;
    }
    SIMPLEX_NO_REACH("Unhandled open flag.");
    return 0;
}

static inline DWORD
translate_share_flag(PlatformFilesystemShareFlag share_flag)
{
    switch (share_flag)
    {
        case PlatformFilesystemShareFlag_ShareRead:         return FILE_SHARE_READ;
        case PlatformFilesystemShareFlag_ShareWrite:        return FILE_SHARE_WRITE;
        case PlatformFilesystemShareFlag_ShareReadWrite:    return FILE_SHARE_READ | FILE_SHARE_WRITE;
    }
    SIMPLEX_NO_REACH("Unhandled share flag.");
    return 0;
}

static inline DWORD
translate_creation_flag(PlatformFilesystemCreationFlag creation_flag)
{
    switch (creation_flag)
    {
        case PlatformFilesystemCreationFlag_CreateAlways:   return CREATE_ALWAYS;
        case PlatformFilesystemCreationFlag_CreateNew:      return CREATE_NEW;
        case PlatformFilesystemCreationFlag_OpenAlways:     return OPEN_ALWAYS;
        case PlatformFilesystemCreationFlag_OpenOnly:       return OPEN_EXISTING;
        case PlatformFilesystemCreationFlag_TruncateOnly:   return TRUNCATE_EXISTING;
    }
    SIMPLEX_NO_REACH("Unhandled creation flag.");
    return 0;
}

// NOTE(Chris): Folds a Win32 error code into our platform-agnostic result enum so
//              that callers never have to reason about Win32 specifics.
static inline PlatformFilesystemResult
translate_last_error(DWORD error_code)
{
    switch (error_code)
    {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_INVALID_NAME:        return PlatformFilesystemResult_PathInvalid;
        case ERROR_SHARING_VIOLATION:
        case ERROR_LOCK_VIOLATION:      return PlatformFilesystemResult_PathUnavailable;
        case ERROR_ACCESS_DENIED:       return PlatformFilesystemResult_PathAccessInvalid;
        default:                        return PlatformFilesystemResult_PathInvalid;
    }
}

// NOTE(Chris): The public file_handle_t is simply the address of the book-keeping
//              record, so resolving it is a cast - no map lookup, no lock.
static inline spx::fs::win32_file_handle*
as_internal_handle(file_handle_t handle)
{
    return (spx::fs::win32_file_handle*)handle;
}

static inline bool
query_native_size(HANDLE native_handle, uint64_t *out_size)
{
    LARGE_INTEGER size = {};
    if (!GetFileSizeEx(native_handle, &size)) return false;
    *out_size = (uint64_t)size.QuadPart;
    return true;
}

// NOTE(Chris): GetModuleFileNameA truncates silently when the buffer is too small,
//              so we grow until the full path fits to support long paths.
static spx::dynamic_string<char>
query_executable_path()
{
    DWORD capacity = MAX_PATH;
    while (true)
    {

        char *raw = (char*)simplex_memory_alloc(capacity);
        DWORD length = GetModuleFileNameA(NULL, raw, capacity);

        if (length == 0)
        {
            simplex_memory_free(raw);
            return spx::dynamic_string<char>();
        }

        // A return value strictly less than the capacity means the path fit.
        if (length < capacity)
        {
            spx::dynamic_string<char> result(raw, length);
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
                if (full_path[i] == '\\' || full_path[i] == '/') last_separator = i;
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

        DWORD required = GetCurrentDirectoryA(0, NULL);
        if (required == 0)
        {
            cached_working_directory.clear();
            return spx::string_view<char>(cached_working_directory.c_str(), cached_working_directory.size());
        }

        char *raw = (char*)simplex_memory_alloc(required);
        DWORD length = GetCurrentDirectoryA(required, raw);
        cached_working_directory = spx::dynamic_string<char>(raw, length);
        simplex_memory_free(raw);

        return spx::string_view<char>(cached_working_directory.c_str(), cached_working_directory.size());

    }

    PlatformFilesystemResult
    path_is_file(spx::string_view<char> path)
    {

        spx::dynamic_string<char> native_path = make_null_terminated(path);
        DWORD attributes = GetFileAttributesA(native_path.c_str());

        if (attributes == INVALID_FILE_ATTRIBUTES) return PlatformFilesystemResult_PathInvalid;
        if (attributes & FILE_ATTRIBUTE_DIRECTORY) return PlatformFilesystemResult_PathNotFile;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    path_is_directory(spx::string_view<char> path)
    {

        spx::dynamic_string<char> native_path = make_null_terminated(path);
        DWORD attributes = GetFileAttributesA(native_path.c_str());

        if (attributes == INVALID_FILE_ATTRIBUTES) return PlatformFilesystemResult_PathInvalid;
        if (attributes & FILE_ATTRIBUTE_DIRECTORY) return PlatformFilesystemResult_OK;
        return PlatformFilesystemResult_PathNotDirectory;

    }

    PlatformFilesystemResult
    path_is_valid(spx::string_view<char> path)
    {

        spx::dynamic_string<char> native_path = make_null_terminated(path);
        DWORD attributes = GetFileAttributesA(native_path.c_str());

        if (attributes == INVALID_FILE_ATTRIBUTES) return PlatformFilesystemResult_PathInvalid;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    file_is_ready(spx::string_view<char> path)
    {

        spx::dynamic_string<char> native_path = make_null_terminated(path);

        DWORD attributes = GetFileAttributesA(native_path.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES) return PlatformFilesystemResult_PathInvalid;
        if (attributes & FILE_ATTRIBUTE_DIRECTORY) return PlatformFilesystemResult_PathNotFile;

        // A successful (non-shared) open is the closest signal that the file can be used.
        HANDLE probe = CreateFileA(native_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (probe == INVALID_HANDLE_VALUE)
        {
            DWORD error_code = GetLastError();
            if (error_code == ERROR_SHARING_VIOLATION || error_code == ERROR_LOCK_VIOLATION)
                return PlatformFilesystemResult_PathUnavailable;
            return PlatformFilesystemResult_FileNotReady;
        }

        CloseHandle(probe);
        return PlatformFilesystemResult_OK;

    }

    spx::dynamic_string<char>
    canonicalize_path(spx::string_view<char> path)
    {

        spx::dynamic_string<char> native_path = make_null_terminated(path);

        DWORD required = GetFullPathNameA(native_path.c_str(), 0, NULL, NULL);
        if (required == 0) return native_path;

        char *raw = (char*)simplex_memory_alloc(required);
        DWORD length = GetFullPathNameA(native_path.c_str(), required, raw, NULL);
        spx::dynamic_string<char> result(raw, length);
        simplex_memory_free(raw);

        return result;

    }

    PlatformFilesystemResult
    get_file_size(spx::string_view<char> path, size_t *file_size)
    {

        SIMPLEX_CHECK_PTR(file_size);

        spx::dynamic_string<char> native_path = make_null_terminated(path);

        WIN32_FILE_ATTRIBUTE_DATA attribute_data = {};
        if (!GetFileAttributesExA(native_path.c_str(), GetFileExInfoStandard, &attribute_data))
            return translate_last_error(GetLastError());

        if (attribute_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            return PlatformFilesystemResult_PathNotFile;

        ULARGE_INTEGER size = {};
        size.HighPart = attribute_data.nFileSizeHigh;
        size.LowPart  = attribute_data.nFileSizeLow;
        *file_size = (size_t)size.QuadPart;

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

        HANDLE native_handle = CreateFileA(native_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (native_handle == INVALID_HANDLE_VALUE)
            return translate_last_error(GetLastError());

        uint64_t file_size = 0;
        if (!query_native_size(native_handle, &file_size))
        {
            CloseHandle(native_handle);
            return PlatformFilesystemResult_FileNotReady;
        }

        if (file_size > buffer_size)
        {
            CloseHandle(native_handle);
            return PlatformFilesystemResult_InsufficientBufferSize;
        }

        uint64_t total_read = 0;
        uint8_t *cursor = (uint8_t*)buffer;
        while (total_read < file_size)
        {
            uint64_t remaining = file_size - total_read;
            DWORD to_read = (remaining > MAXDWORD) ? MAXDWORD : (DWORD)remaining;
            DWORD chunk_read = 0;
            if (!ReadFile(native_handle, cursor + total_read, to_read, &chunk_read, NULL))
            {
                CloseHandle(native_handle);
                return PlatformFilesystemResult_FileNotReady;
            }
            if (chunk_read == 0) break; // Reached EOF earlier than reported size.
            total_read += chunk_read;
        }

        CloseHandle(native_handle);
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

        HANDLE native_handle = CreateFileA(native_path.c_str(), GENERIC_WRITE, 0, NULL,
                                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (native_handle == INVALID_HANDLE_VALUE)
            return translate_last_error(GetLastError());

        uint64_t total_written = 0;
        const uint8_t *cursor = (const uint8_t*)buffer;
        while (total_written < buffer_size)
        {
            uint64_t remaining = buffer_size - total_written;
            DWORD to_write = (remaining > MAXDWORD) ? MAXDWORD : (DWORD)remaining;
            DWORD chunk_written = 0;
            if (!WriteFile(native_handle, cursor + total_written, to_write, &chunk_written, NULL))
            {
                CloseHandle(native_handle);
                *bytes_written = (size_t)total_written;
                return PlatformFilesystemResult_FileNotReady;
            }
            total_written += chunk_written;
        }

        CloseHandle(native_handle);
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

        DWORD desired_access = translate_open_flag(open_flag);
        DWORD share_mode     = translate_share_flag(share_flag);
        DWORD disposition    = translate_creation_flag(creation_flag);

        HANDLE native_handle = CreateFileA(native_path.c_str(), desired_access, share_mode, NULL,
                                           disposition, FILE_ATTRIBUTE_NORMAL, NULL);
        if (native_handle == INVALID_HANDLE_VALUE)
        {
            DWORD error_code = GetLastError();
            spx::logger::dispatch_warning_log("Failed to open file {} (Win32 error {}).", native_path.c_str(), error_code);
            return translate_last_error(error_code);
        }

        win32_file_handle record   = {};
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
        win32_file_handle &stored = handles.insert(native_handle, std::move(record));

        *handle = (file_handle_t)&stored;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    close_file(file_handle_t *handle)
    {

        SIMPLEX_CHECK_PTR(handle);
        if (*handle == NULL) return PlatformFilesystemResult_InvalidHandle;

        win32_file_handle *record = as_internal_handle(*handle);
        HANDLE native_handle = record->handle;

        std::unique_lock<std::shared_mutex> lock(handles_mutex);

        // Guard against double-close: only act if the handle is still registered.
        if (handles.find(native_handle) == NULL) return PlatformFilesystemResult_InvalidHandle;

        CloseHandle(native_handle);
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

        win32_file_handle *record = as_internal_handle(handle);
        if (record == NULL) return PlatformFilesystemResult_InvalidHandle;

        HANDLE native_handle = record->handle;
        uint64_t offset      = record->pointer;
        uint64_t total_read  = 0;
        uint8_t *cursor      = (uint8_t*)buffer;

        // The offset travels in the OVERLAPPED struct, so each read is a single syscall
        // and never touches the shared OS file pointer - our tracked pointer is authoritative.
        while (total_read < read_size)
        {
            uint64_t position = offset + total_read;
            uint64_t remaining = read_size - total_read;
            DWORD to_read = (remaining > MAXDWORD) ? MAXDWORD : (DWORD)remaining;

            OVERLAPPED overlapped = {};
            overlapped.Offset     = (DWORD)(position & 0xFFFFFFFFull);
            overlapped.OffsetHigh = (DWORD)(position >> 32);

            DWORD chunk_read = 0;
            if (!ReadFile(native_handle, cursor + total_read, to_read, &chunk_read, &overlapped))
            {
                if (GetLastError() == ERROR_HANDLE_EOF) break; // Offset at or past EOF.
                return PlatformFilesystemResult_PathUnavailable;
            }
            if (chunk_read == 0) break; // EOF reached.
            total_read += chunk_read;
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

        win32_file_handle *record = as_internal_handle(handle);
        if (record == NULL) return PlatformFilesystemResult_InvalidHandle;

        HANDLE native_handle    = record->handle;
        uint64_t offset         = record->pointer;
        uint64_t total_written  = 0;
        const uint8_t *cursor   = (const uint8_t*)buffer;

        while (total_written < write_size)
        {
            uint64_t position = offset + total_written;
            uint64_t remaining = write_size - total_written;
            DWORD to_write = (remaining > MAXDWORD) ? MAXDWORD : (DWORD)remaining;

            OVERLAPPED overlapped = {};
            overlapped.Offset     = (DWORD)(position & 0xFFFFFFFFull);
            overlapped.OffsetHigh = (DWORD)(position >> 32);

            DWORD chunk_written = 0;
            if (!WriteFile(native_handle, cursor + total_written, to_write, &chunk_written, &overlapped))
            {
                record->pointer = (size_t)(offset + total_written);
                if (record->pointer > record->size) record->size = record->pointer;
                *amount_written = (size_t)total_written;
                return PlatformFilesystemResult_PathUnavailable;
            }
            total_written += chunk_written;
        }

        record->pointer = (size_t)(offset + total_written);
        if (record->pointer > record->size) record->size = record->pointer; // Track growth.
        *amount_written = (size_t)total_written;

        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    move_file_pointer(file_handle_t handle, int64_t relative)
    {

        win32_file_handle *record = as_internal_handle(handle);
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

        win32_file_handle *record = as_internal_handle(handle);
        if (record == NULL) return PlatformFilesystemResult_InvalidHandle;

        if (record->open_flag == PlatformFilesystemOpenFlag_Read && absolute > record->size)
            return PlatformFilesystemResult_EOF;

        record->pointer = (size_t)absolute;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    set_file_pointer_at_bof(file_handle_t handle)
    {

        win32_file_handle *record = as_internal_handle(handle);
        if (record == NULL) return PlatformFilesystemResult_InvalidHandle;

        record->pointer = 0;
        return PlatformFilesystemResult_OK;

    }

    PlatformFilesystemResult
    set_file_pointer_at_eof(file_handle_t handle)
    {

        win32_file_handle *record = as_internal_handle(handle);
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

        win32_file_handle *record = as_internal_handle(handle);
        if (record == NULL) return PlatformFilesystemResult_InvalidHandle;

        *location = record->pointer;
        return PlatformFilesystemResult_OK;

    }

}
