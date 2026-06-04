#pragma once
#include <utils/defs.hpp>

namespace spx::package
{

    /// @brief The package type describes what exists in the package.
    enum SimplexPackageDataType : uint64_t
    {

        SimplexPackageDataType_V1_Invalid,             /// Invalid entry, application should throw error.
        SimplexPackageDataType_V1_PNG,                 /// PNG image file.
        SimplexPackageDataType_V1_JPG,                 /// JPG/JPEG image file.
        SimplexPackageDataType_V1_BMP,                 /// BMP image file.
        SimplexPackageDataType_V1_PNM,                 /// PNM image file.
        SimplexPackageDataType_V1_WAV,                 /// Wave audio file.
        SimplexPackageDataType_V1_MP3,                 /// MP3 audio file.
        SimplexPackageDataType_V1_OGG,                 /// OGG audio file.
        SimplexPackageDataType_V1_FLAC,                /// FLAC loss-less audio file.
        SimplexPackageDataType_V1_OBJ,                 /// Wavefront OBJ mesh file.
        SimplexPackageDataType_V1_MTL,                 /// Wavefront MTL material file.
        

        SimplexPackageDataType_Count,                  /// Used to determine how many package data types there is.
    };

    /// @brief The package header of a simplex package.
    struct simplex_package_header_v1
    {

        char magic[3];

        uint8_t flags;

        union
        {
            uint32_t version_packed;
            struct
            {
                uint8_t _version_padding;
                uint8_t major;
                uint8_t minor;
                uint8_t patch;
            };
        };

        uint64_t identifier;                /// Package identifier.
        uint64_t size;                      /// Total file size, dummy-check.
        uint64_t header_size;               /// Header size, matches sizeof(simplex_package_header_v1)
        uint64_t data_entry_size;           /// Data entry size, matches sizeof(simplex_package_data_v1)
        uint32_t data_entry_count;          /// Number of data entries to process.
        uint32_t data_entry_offset;         /// Offset to the first data entry, contiguously.

    };

    /// @brief A package data entry descriptor.
    struct simplex_package_data_v1
    {

        uint64_t compressed_checksum;       /// xxHash3
        uint64_t uncompressed_checksum;     /// xxHash3
        uint64_t package_identifier;        /// Origin location of asset, if different from this, it is
                                            /// in another package file. In most cases it is local to the
                                            /// package itself.
        uint64_t package_type;              /// Package type, generally just the file format. Internal formats
                                            /// such as pre-formatted meshes are also marked here. This is for
                                            /// the asset management system for registration.
        uint64_t size_compressed;           /// Compressed size. This value is zero if it isn't 
                                            /// local to this package.
        uint64_t size_uncompressed;         /// Uncompressed size. This value is zero if it isn't
                                            /// local to this package.
        uint64_t offset;                    /// Data offset in file. This value is zero if it isn't local
                                            /// to this package.

    };

}
