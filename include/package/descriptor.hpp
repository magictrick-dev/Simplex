#pragma once
#include <utils/defs.hpp>
#include <vendor/nlohmann/json.hpp>

namespace spx::package
{

    // NOTE(Chris): The Simplex package format is designed to tightly package and compress
    //              streamable assets into a uniform file.
    //
    //  [SPX_PACKAGE_HEADER][SPX_PACKAGE_DESCRIPTOR ...][ Data 1 ...][ Data 2 ...]...
    //  [ Data n-1 ...][ Data n ...]
    //
    //  There is no rules as to what is contained in the package; generally it will be
    //  textures, meshes, and such. For advances cases, (eg. custom game engines) you can
    //  package in-memory DLL files. The main advantage of this format is that you can isolate
    //  certain sections of your scenes (or one scene to another).
    //
    //  The package format compresses individual data sections, not the entire package itself.
    //  Move the read pointer to the head of the data you wish to stream and then begin to pull
    //  compressed blocks out with LZ4.
    //

#   pragma pack(push, 1)
    struct spx_package_header_v1
    {

        union
        {
            uint64_t magic_packed;

            struct
            {
                char magic_bytes[3];
                char magic_always_zero[7];
            };

        };

        union
        {

            uint32_t format_version;
            struct
            {
                uint16_t format_version_always_zero;
                uint8_t minor;
                uint8_t major;
            };

        };

        union
        {

            uint32_t package_version;
            struct
            {
                uint8_t version_always_zero;
                uint8_t build;
                uint8_t minor;
                uint8_t major;
            };

        };

        uint64_t package_size;      // File size, should mirror literal file size.
        uint64_t descriptor_size;   // JSON descriptor size.
        uint64_t data_size;         // Total packed data size.
        uint64_t descriptor_offset; // The JSON descriptor offset, in bytes from head-of-file. 
        uint64_t data_offset;       // The data offset, in bytes from head-of-file.

    };
#   pragma pack(pop)

#   pragma pack(push, 1)
    struct spx_package_binary_v1
    {

        char name[128];
        size_t binary_size;
        size_t relative_offset;

    };
#   pragma pop

}
