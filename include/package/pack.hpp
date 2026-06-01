#pragma once
#include <utils/defs.hpp>

namespace spx::package
{

#   pragma pack(push, 1)
    struct spx_header
    {

        union
        {
            uint64_t magic_packed;

            struct
            {
                char magic_bytes[3];
                char magic_pad;
                char text_format; // Always 'JSON', until a new format is made.
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

};
