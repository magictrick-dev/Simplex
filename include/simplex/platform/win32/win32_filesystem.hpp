#pragma once
#include <utils/defs.hpp>
#include <simplex/platform/filesystem.hpp>
#if defined(_WIN32)
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   include <windows.h>

namespace spx::fs
{

    // NOTE(Chris): The address of this record is the public file_handle_t. It lives
    //              in the registry map (address-stable across rehashes), so read/write/
    //              seek calls dereference the handle directly and mutate the tracked
    //              pointer with no map lookup and no locking. Sharing a single handle
    //              across threads is therefore not thread-safe by contract; the expected
    //              usage is to dispatch a handle to a single owning thread.
    struct win32_file_handle
    {

        HANDLE handle = NULL;
        size_t pointer = 0;
        size_t size = 0;

        PlatformFilesystemCreationFlag creation_flag;
        PlatformFilesystemOpenFlag open_flag;
        PlatformFilesystemShareFlag share_flag;

        spx::dynamic_string<char> path;

    };

}

#endif