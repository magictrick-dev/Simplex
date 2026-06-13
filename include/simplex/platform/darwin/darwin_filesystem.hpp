#pragma once
#include <utils/defs.hpp>
#include <simplex/platform/filesystem.hpp>
#if defined(__APPLE__)

namespace spx::fs
{

    // NOTE(Chris): The address of this record is the public file_handle_t. It lives
    //              in the registry map (address-stable across rehashes), so read/write/
    //              seek calls dereference the handle directly and mutate the tracked
    //              pointer with no map lookup and no locking. Sharing a single handle
    //              across threads is therefore not thread-safe by contract; the expected
    //              usage is to dispatch a handle to a single owning thread.
    //
    //              The native handle is a POSIX file descriptor. All positional I/O is
    //              done with pread/pwrite, so the descriptor's own seek offset is never
    //              used and the tracked pointer below is authoritative.
    struct darwin_file_handle
    {

        int handle = -1;
        size_t pointer = 0;
        size_t size = 0;

        PlatformFilesystemCreationFlag creation_flag;
        PlatformFilesystemOpenFlag open_flag;
        PlatformFilesystemShareFlag share_flag;

        spx::dynamic_string<char> path;

    };

}

#endif
