#pragma once
#include <utils/defs.hpp>
#include <simplex/platform/filesystem.hpp>
#if defined(_WIN32)
#include <windows.h>

namespace spx::fs
{

    struct win32_file_handle
    {

        HANDLE handle = NULL;
        size_t pointer = 0;

        PlatformFilesystemCreationFlag creation_flag;
        PlatformFilesystemOpenFlag open_flag;
        PlatformFilesystemShareFlag share_flag;

        spx::string_view<char> path;

    };

}

#endif