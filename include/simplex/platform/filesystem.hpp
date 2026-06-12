#pragma once
#include <utils/defs.hpp>
#include <simplex/string_view.hpp>
#include <simplex/dynamic_string.hpp>

enum PlatformFilesystemResult : uint32_t
{
    PlatformFilesystemResult_OK,                        // Return result is valid.
    PlatformFilesystemResult_PathInvalid,               // Does not exist
    PlatformFilesystemResult_PathUnavailable,           // Locked, but exists.
    PlatformFilesystemResult_PathAccessInvalid,         // Invalid shared access.
    PlatformFilesystemResult_InsufficientBufferSize,    // Buffer size is too small.
    PlatformFilesystemResult_InvalidHandle,             // File handle is invalid.
    PlatformFilesystemResult_EOF,                       // Read operation reached EOF.
    PlatformFilesystemResult_FileNotReady,              // File is not ready query.
    PlatformFilesystemResult_PathNotFile,               // Path not file.
    PlatformFilesystemResult_PathNotDirectory,          // Path not directory.
};

enum PlatformFilesystemOpenFlag : uint32_t
{
    PlatformFilesystemOpenFlag_Read,                    // Handle allows reading.
    PlatformFilesystemOpenFlag_Write,                   // Handle allows writing.
    PlatformFilesystemOpenFlag_ReadWrite                // Handle allows both reading and writing.
};

enum PlatformFilesystemShareFlag : uint32_t
{
    PlatformFilesystemShareFlag_ShareRead,              // Shares read access with other handles.
    PlatformFilesystemShareFlag_ShareWrite,             // Shares write access with other handles.
    PlatformFilesystemShareFlag_ShareReadWrite,         // Shares read & write access with other handles.
};

enum PlatformFilesystemCreationFlag : uint32_t
{
    PlatformFilesystemCreationFlag_CreateAlways,        // Truncates file if it exists.
    PlatformFilesystemCreationFlag_CreateNew,           // Create only if new, fail otherwise.
    PlatformFilesystemCreationFlag_OpenAlways,          // Opens file or creates one.
    PlatformFilesystemCreationFlag_OpenOnly,            // Opens an existing file only.
    PlatformFilesystemCreationFlag_TruncateOnly,        // Opens an existing file and truncates only.
};

typedef void* file_handle_t;

namespace spx::fs
{

    /// @brief Returns the executable directory of the application.
    /// @return The path of the executable directory.
    ///
    /// The executable directory is immutable and the result is cached after the
    /// first call. The actual resulting directory is dependent on the executable's
    /// directory; changing it will obviously change the relative layout of the executable.
    spx::string_view<char> get_executable_directory();

    /// @brief Returns the current working directory.
    /// @return The path of the current working directory.
    ///
    /// The current working directory queries are cached per-thread. Since the CWD
    /// is technically mutable at runtime, each subsequent calls may not be predictably
    /// the same.
    spx::string_view<char> get_working_directory();

    /// @brief Checks if a given path is a file.
    /// @param path The path to check.
    /// @return OK if the path is a file, other values indicate the failure.
    PlatformFilesystemResult path_is_file(spx::string_view<char> path);

    /// @brief Checks if a given path is a directory.
    /// @param path The path to check.
    /// @return OK if the path is a directory, other values indicate failure.
    PlatformFilesystemResult path_is_directory(spx::string_view<char> path);

    /// @brief Checks if a given path is valid.
    /// @param path The path to check.
    /// @return OK if the path is a directory or file.
    PlatformFilesystemResult path_is_valid(spx::string_view<char> path);

    /// @brief Checks if a file can be opened or not.
    /// @param path The path to check.
    /// @return OK if the file can be opened (doesn't guarantee, though).
    PlatformFilesystemResult file_is_ready(spx::string_view<char> path);

    /// @brief Transforms a path to a canonical system path.
    /// @param path The path to transform.
    /// @return The transformed path. Doesn't guarantee the path exists, just canonical.
    spx::dynamic_string<char> canonicalize_path(spx::string_view<char> path);

    /// @brief Gets the file's size.
    /// @param path The path to query.
    /// @param file_size Output parameter of the file size.
    /// @return OK if the output parameter(s) are set.
    PlatformFilesystemResult get_file_size(spx::string_view<char> path, size_t *file_size);

    /// @brief Reads an entire file into the provided buffer.
    /// @param path The path to the file to read.
    /// @param buffer The buffer pointer to read into.
    /// @param buffer_size The size of the buffer the user guarantees has a minimum size of.
    /// @param bytes_read Outputs the number of bytes read into the buffer.
    /// @return OK if the read trully read all bytes into the buffer.
    PlatformFilesystemResult read_entire_file(spx::string_view<char> path, 
                                              void *buffer, 
                                              size_t buffer_size,
                                              size_t *bytes_read);

    /// @brief Writes an entire file from the provided buffer.
    /// @param path The path to the file to write.
    /// @param buffer The buffer pointer to read from.
    /// @param buffer_size The size of the buffer the user wants to read out of.
    /// @param bytes_read Outputs the number of bytes written to the file.
    /// @return OK if the write trully wrote all bytes into the file.
    PlatformFilesystemResult write_entire_file(spx::string_view<char> path, 
                                               void *buffer, 
                                               size_t buffer_size,
                                               size_t *bytes_written);
    
    
    /// @brief Opens a file and returns a handle to it.
    /// @param handle The output handle.
    /// @param file_path The path to open.
    /// @param open_flag The opening flag (see enum).
    /// @param share_flag The sharing flag (see enum).
    /// @param creation_flag The creation flag (see enum).
    /// @return OK if the file was openned as expected.
    PlatformFilesystemResult open_file(file_handle_t *handle,
                                       spx::string_view<char> file_path,
                                       PlatformFilesystemOpenFlag open_flag,
                                       PlatformFilesystemShareFlag share_flag,
                                       PlatformFilesystemCreationFlag creation_flag);

    /// @brief Closes a file handle. Mutates the handle once closed to NULL.
    /// @param handle The handle passed in.
    /// @return OK if the close was successful.
    PlatformFilesystemResult close_file(file_handle_t *handle);

    /// @brief Reads from a file into a buffer.
    /// @param handle The handle to read from.
    /// @param buffer The buffer to read into.
    /// @param read_size The amount of bytes to stream into the file.
    /// @param amount_read The amount of bytes actually streamed out of the file. May be less than
    ///                    what was requested if EOF. 
    /// @return OK if the read succeeded.
    ///
    /// File reading can fail for a number of reasons; the user is expected to inspect the
    /// result to see if the read failed because it was a partial read or not.
    PlatformFilesystemResult read_file(file_handle_t *handle, void *buffer, size_t read_size, size_t *amount_read);

    /// @brief Writes to a file from a buffer.
    /// @param handle The handle to write to.
    /// @param buffer The buffer to write from.
    /// @param write_size The amount of bytes to write.
    /// @param amount_written The amount of bytes actually written. Typically should be
    ///                       one-to-one unless a failure occured.
    /// @return OK if the write succeeded.
    PlatformFilesystemResult write_file(file_handle_t *handle, void *buffer, size_t write_size, size_t *amount_written);

    /// @brief Moves the internal file pointer.
    /// @param handle The handle to adjust the file pointer to.
    /// @param relative Positive/negative movement value.
    /// @return OK if the movement was valid and within range.
    ///
    /// May fail if the movement went beyond the start or EOF for reading, or beyond the start
    /// for writing. Moving beyond EOF during writing pads zero bytes when a write occurs.
    PlatformFilesystemResult move_file_pointer(file_handle_t *handle, int64_t relative);

    /// @brief Sets the file pointer to an absolute position.
    /// @param handle The handle to adjust the file pointer.
    /// @param absolute The absolute position to set the file pointer to.
    /// @return OK if the position is within a valid range for a given read/write operation.
    PlatformFilesystemResult set_file_pointer(file_handle_t *handle, uint64_t absolute);

    /// @brief Sets the file pointer to the beginning of the file.
    /// @param handle The handle to adjust the file pointer.
    /// @return OK if the movement succeeded. (Usually does.)
    PlatformFilesystemResult set_file_pointer_at_bof(file_handle_t *handle);

    /// @brief Sets the file pointer to the end of the file.
    /// @param handle The handle to adjust the file pointer.
    /// @return OK if the movement succeeded. (Usually does.)
    PlatformFilesystemResult set_file_pointer_at_eof(file_handle_t *handle);

}