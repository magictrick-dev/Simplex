#pragma once
#include <utils/defs.hpp>
#include <simplex/dynamic_array.hpp>
#include <simplex/string_view.hpp>
#include <simplex/dynamic_string.hpp>

namespace spx::package
{

    /// @brief Handles packing, either by a directory with a manifest.json or by manually
    ///        supplying files.
    class packer_v1
    {

        public:
             packer_v1();
            ~packer_v1();

            bool add_resource(spx::string_view<char> resource_path);
            bool add_resources_from_manifest(spx::string_view<char> manifest_path);

        private:
            bool verify_path(spx::string_view<char> path);

        private:
            spx::dynamic_array<spx::dynamic_string<char>> file_paths;

    };

};
