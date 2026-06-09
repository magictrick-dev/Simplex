#pragma once
#include <utils/defs.hpp>
#include <simplex/string_view.hpp>
#include <simplex/dynamic_string.hpp>

#if defined(_WIN32)
#   include <windows.h>
#elif defined(__APPLE__)
#   include <unistd.h>
#   include <limits.h>
#   include <sys/stat.h>
#   include <mach-o/dyld.h>
#elif defined(__unix__)
#   include <unistd.h>
#   include <limits.h>
#   include <sys/stat.h>
#else
#   pragma error "Undefined platform implementation for spx::filesystem."
#endif

// SIMPLEX_MAX_PATH is the size of the scratch buffer handed to path-related system calls,
// taken from the platform's own maximum. It is NOT a cap on the paths this module stores
// or returns: results live in spx::dynamic_string and grow as needed, so a path longer
// than the scratch size is handled (the syscall is retried on a heap buffer) rather than
// truncated or asserted.
#if defined(_WIN32)
#   define SIMPLEX_MAX_PATH MAX_PATH
#elif defined(PATH_MAX)
#   define SIMPLEX_MAX_PATH PATH_MAX
#else
#   define SIMPLEX_MAX_PATH 4096
#endif

namespace spx::fs
{

    inline spx::string_view<char>
    get_executable_directory()
    {

        // The executable's path is fixed for the lifetime of the process, so resolve it
        // once on first call and cache it. The cache is thread_local, so each thread
        // initializes its own copy with no shared state to race on, and because the store
        // outlives every call the returned view is valid for the whole program.
        static thread_local spx::dynamic_string<char> path;
        static thread_local bool initialized = false;

        if (initialized == false)
        {

            initialized = true;

            char buffer[SIMPLEX_MAX_PATH + 1];
            size_t length = 0;

#           if defined(_WIN32)
                length = (size_t)GetModuleFileNameA(NULL, buffer, SIMPLEX_MAX_PATH);
#           elif defined(__APPLE__)
                uint32_t size = SIMPLEX_MAX_PATH + 1;
                if (_NSGetExecutablePath(buffer, &size) == 0)
                    length = spx::dynamic_string<char>::str_length(buffer);
#           elif defined(__unix__)
                ssize_t written = readlink("/proc/self/exe", buffer, SIMPLEX_MAX_PATH);
                if (written > 0) length = (size_t)written;
#           else
#               pragma error "Undefined platform implementation for spx::filesystem"
#           endif

            // Trim the trailing executable name (and its separator) to leave the directory.
            while (length > 0 && buffer[length - 1] != '/' && buffer[length - 1] != '\\')
                --length;
            if (length > 0) --length;

            path.append(buffer, length);

        }

        return path;

    }

    inline spx::string_view<char>
    get_working_directory()
    {

        // Unlike the executable path, the working directory is mutable (chdir /
        // SetCurrentDirectory), so this is queried live on every call and must NOT be
        // cached. The returned view aliases this thread-local store, so it stays valid
        // until the next call on the same thread; copy it out if you need it longer.
        static thread_local spx::dynamic_string<char> path;
        path.clear();

        char buffer[SIMPLEX_MAX_PATH + 1];
        size_t length = 0;

#       if defined(_WIN32)
            length = (size_t)GetCurrentDirectoryA(SIMPLEX_MAX_PATH + 1, buffer);
#       elif defined(__unix__) || defined(__APPLE__)
            if (getcwd(buffer, SIMPLEX_MAX_PATH + 1) != NULL)
                length = spx::dynamic_string<char>::str_length(buffer);
#       else
#           pragma error "Undefined platform implementation for spx::filesystem"
#       endif

        path.append(buffer, length);
        return path;

    }

    inline bool
    path_is_file(spx::string_view<char> path)
    {

        // The view is not guaranteed null-terminated, so copy into a terminated, growable
        // buffer before handing the path off to the platform C API. The path length is not
        // capped: dynamic_string sizes itself to whatever the caller passed.
        spx::dynamic_string<char> cpath(path);

#       if defined(_WIN32)
            DWORD attributes = GetFileAttributesA(cpath.c_str());
            return (attributes != INVALID_FILE_ATTRIBUTES) &&
                   ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0);
#       elif defined(__unix__) || defined(__APPLE__)
            struct stat info;
            if (stat(cpath.c_str(), &info) != 0) return false;
            return S_ISREG(info.st_mode);
#       else
#           pragma error "Undefined platform implementation for spx::filesystem"
#       endif

    }

    inline bool
    path_is_directory(spx::string_view<char> path)
    {

        // The view is not guaranteed null-terminated, so copy into a terminated, growable
        // buffer before handing the path off to the platform C API. The path length is not
        // capped: dynamic_string sizes itself to whatever the caller passed.
        spx::dynamic_string<char> cpath(path);

#       if defined(_WIN32)
            DWORD attributes = GetFileAttributesA(cpath.c_str());
            return (attributes != INVALID_FILE_ATTRIBUTES) &&
                   ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
#       elif defined(__unix__) || defined(__APPLE__)
            struct stat info;
            if (stat(cpath.c_str(), &info) != 0) return false;
            return S_ISDIR(info.st_mode);
#       else
#           pragma error "Undefined platform implementation for spx::filesystem"
#       endif

    }

    inline bool
    path_is_valid(spx::string_view<char> path)
    {
        return (path_is_file(path) || path_is_directory(path));
    }

    inline spx::dynamic_string<char>
    canonicalize_path(spx::string_view<char> path)
    {

        // Produces an absolute, lexically normalized path (collapsing "." / ".." segments)
        // like std::filesystem::weakly_canonical: the path is NOT required to exist. This
        // is purely lexical, so unlike realpath it does not resolve symlinks.

        // The view is not guaranteed null-terminated, so copy into a terminated, growable
        // buffer before handing the path off to the platform C API.
        spx::dynamic_string<char> cpath(path);

#       if defined(_WIN32)
            // GetFullPathNameA makes the path absolute relative to the current directory and
            // collapses "." / ".." without touching the filesystem. When the buffer is large
            // enough it returns the length written (excluding null); when it is too small it
            // returns the required size (including null), so we size exactly and retry on the
            // heap rather than capping the result at the scratch size.
            char buffer[SIMPLEX_MAX_PATH + 1];
            DWORD needed = GetFullPathNameA(cpath.c_str(), sizeof(buffer), buffer, NULL);
            if (needed == 0) return spx::dynamic_string<char>();
            if (needed < sizeof(buffer))
                return spx::dynamic_string<char>(buffer, (size_t)needed);

            char *heap = (char*)malloc(needed);
            DWORD written = GetFullPathNameA(cpath.c_str(), needed, heap, NULL);
            spx::dynamic_string<char> result(heap, (size_t)written);
            free(heap);
            return result;
#       elif defined(__unix__) || defined(__APPLE__)
            // No libc call does lexical-absolute normalization without requiring existence
            // (realpath insists the path exist), so build it by hand: anchor a relative path
            // to the working directory, then walk segments collapsing "." and "..".
            spx::dynamic_string<char> absolute;
            if (cpath.size() == 0 || cpath[0] != '/')
            {
                absolute.append(get_working_directory());
                absolute += '/';
            }
            absolute.append(cpath);

            const char *d = absolute.data();
            const size_t n = absolute.size();

            spx::dynamic_string<char> result;
            result += '/';

            size_t i = 0;
            while (i < n)
            {
                while (i < n && d[i] == '/') ++i;       // skip run of separators
                const size_t start = i;
                while (i < n && d[i] != '/') ++i;        // span one segment
                const size_t len = i - start;

                if (len == 0) continue;
                if (len == 1 && d[start] == '.') continue;
                if (len == 2 && d[start] == '.' && d[start + 1] == '.')
                {
                    // Drop the previous segment, but never ascend above root.
                    while (result.size() > 1 && result.back() != '/') result.pop_back();
                    if (result.size() > 1) result.pop_back();
                    continue;
                }

                if (result.back() != '/') result += '/';
                result.append(&d[start], len);
            }

            return result;
#       else
#           pragma error "Undefined platform implementation for spx::filesystem"
#       endif

    }

}
