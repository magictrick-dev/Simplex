#pragma once
#include <string>
#include <filesystem>
#include <unordered_map>
#include <mutex>
#include <atomic>

#include <utils/system/memory_alloc.hpp>

enum ResourceType
{
    ResourceType_Invalid,
    ResourceType_BinaryFile,
};

class ResourceInterface
{
    public:
        inline ResourceInterface(ResourceType resource_type) : type(resource_type) { }
        virtual inline ~ResourceInterface() = default;

        virtual bool is_ready() = 0;
        virtual bool is_available() = 0;

        virtual void load() = 0;
        virtual void discard() = 0;

        inline ResourceType get_resource_type() const { return this->type; }

    private:
        ResourceType type;

};

class FileResourceInterface : public ResourceInterface
{
    public:
        inline FileResourceInterface(ResourceType resource_type) : ResourceInterface(resource_type) { }
        inline virtual ~FileResourceInterface() = default;

        inline std::string get_resource_path() const { return this->canonical_path.string(); }

    protected:
        std::filesystem::path canonical_path;

};

class BinaryFileResource : public FileResourceInterface
{
    public:
        inline BinaryFileResource() : FileResourceInterface(ResourceType_BinaryFile) { }
        inline BinaryFileResource(std::filesystem::path input) 
            : FileResourceInterface(ResourceType_BinaryFile)
        { 
            this->canonical_path = std::filesystem::weakly_canonical(input);
        }

        virtual inline bool is_ready() override { return false; }
        virtual inline bool is_available() override { return false; }
        virtual inline void load() override { }
        virtual inline void discard() override { }

};

enum ResourceManagerResult
{
    ResourceManagerResult_OK,
    ResourceManagerResult_FileNotFound,
};

class ResourceManager
{
    public:
        inline static ResourceManagerResult
        RegisterBinaryFile(std::filesystem::path input)
        {

            std::filesystem::path canonical_path = std::filesystem::weakly_canonical(input);
            if (!std::filesystem::exists(canonical_path)) return ResourceManagerResult_FileNotFound;

            

            return ResourceManagerResult_OK;

        }

    private:
        template <typename T, typename... Args> inline static T*
        CreateResource(Args&&... args)
        {

            // NOTE(Chris): We assume that the front-end API handles any thread-synchronization.
            T* resource = simplex_memory_new<T>(std::forward<Args>(args)...);
            resources.push_back(resources);
            return resource;

        }

    private:
        static inline std::mutex resource_mtx;
        static inline std::vector<ResourceInterface*> resources;
        static inline std::unordered_map<std::filesystem::path, FileResourceInterface*> registered_files;
};