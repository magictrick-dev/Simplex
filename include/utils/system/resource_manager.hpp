#pragma once
#include <string>
#include <filesystem>


enum ResourceManagerResult
{
    ResourceManagerResult_OK,
    ResourceManagerResult_FileNotFound,
    ResourceManagerResult_FileNotAvailable,
    ResourceManagerResult_ResourceNotFound,
    ResourceManagerResult_ResourceExists,
};

union ResourceHandle
{

    uint64_t handle;
    struct
    {
        uint32_t identifier;
        uint32_t generation;
    };

    inline operator uint64_t() const { return handle; };

};

class ResourceManager
{
    public:
        static ResourceManagerResult IsFileResourceRegistered(std::filesystem::path input);

        static ResourceManagerResult RegisterBinaryFile(std::filesystem::path input, ResourceHandle *handle);
        static ResourceManagerResult RegisterTextFile(std::filesystem::path input, ResourceHandle *handle);

};