#include <simplex/platform/win32/win32_filesystem.hpp>
#include <simplex/dynamic_string.hpp>
#include <simplex/hashed_sparse_map.hpp>

#include <shared_mutex>

// NOTE(Chris): When files are added, we associate them with their native handles
//              and file paths. That way we can properly discern who/what they are
//              for debugging.
// TODO(Chris): Add a front-end API to access all active file handles.
static std::shared_mutex handles_mutex;
static spx::hashed_sparse_map<HANDLE, spx::fs::win32_file_handle> handles;
static spx::hashed_sparse_map<HANDLE, spx::dynamic_string<char>> handles_to_paths;


