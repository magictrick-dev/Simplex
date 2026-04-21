#pragma once
#include <utils/defs.hpp>

#if defined(__APPLE__)
#   include <sys/types.h>
#   include <sys/uio.h>
#   include <unistd.h>
#   include <fcntl.h>
#endif

#include <string>
#include <stack>
#include <mutex>
#include <filesystem>
#include <unordered_map>
#include <type_traits>

enum ResourceDescriptorType
{
    ResourceDescriptorType_Binary,
    ResourceDescriptorType_Text,
};

struct IResourceDescriptor
{
    std::string alias;
    std::filesystem::path path;
    ResourceDescriptorType type;
    void   *data_ptr;
    size_t  data_size;
    size_t  file_size;
};

struct BinaryResourceDescriptor : public IResourceDescriptor
{

};

struct TextResourceDescriptor : public IResourceDescriptor
{
    const char *text;
};

enum ResourceManagerResult
{
    ResourceManagerResult_OK,
    ResourceManagerResult_FileNotFound,
    ResourceManagerResult_FileOpenFailed,
    ResourceManagerResult_FileTooLarge,
    ResourceManagerResult_FileReadingFailed,
};

class ResourceManager
{

    public:
        static bool resource_by_path_exists(const std::filesystem::path &path);
        static bool resource_by_alias_exists(const std::string &alias);

        template <typename T> static inline ResourceManagerResult
        load_resource(const std::filesystem::path &path, ResourceDescriptorType as = ResourceDescriptorType_Binary)
        {

            static_assert(std::is_base_of<IResourceDescriptor, T>::value, 
                "Template typename T must inherit IResourceDescriptor.");
            
            const auto canon_path = std::filesystem::weakly_canonical(path);
            if (!std::filesystem::exists(canon_path)) return ResourceManagerResult_FileNotFound;

            const size_t file_size = std::filesystem::file_size(canon_path);          
            const size_t memory_size = file_size + 1; // TODO(Chris): Update to use fixed memory stride.

            // NOTE(Chris): Since, by specification, read can only go up-to INT_MAX, reading large
            //              files requires a read-loop which isn't implemented. This is an extreme
            //              edge case we won't support.
#           if defined(__APPLE__)
                if (memory_size >= INT_MAX) return ResourceManagerResult_FileTooLarge;
#           endif

            void *file_buffer = malloc(memory_size);
#           if defined(__APPLE__)
                int file_descriptor = open(canon_path.c_str(), O_RDONLY);
                if (file_descriptor <= 0)
                {
                    free(file_buffer);
                    return ResourceManagerResult_FileOpenFailed;
                }

                // NOTE(Chris): Enforcing that our read successfully grabs all bytes.
                ssize_t file_read = read(file_descriptor, file_buffer, (int)file_size);
                if (file_read != file_size)
                {
                    free(file_buffer);
                    return ResourceManagerResult_FileReadingFailed;
                }

                close(file_descriptor);
#           endif

            IResourceDescriptor *resource = new T();
            resource->alias = "";
            resource->path = canon_path;
            resource->data_ptr = file_buffer;
            resource->data_size = memory_size;
            resource->file_size = file_size;
            if constexpr (std::is_same<T, TextResourceDescriptor>::value)
            {
                TextResourceDescriptor *casted_resource = static_cast<TextResourceDescriptor*>(resource);
                char *resource_text = static_cast<char*>(casted_resource->data_ptr);
                resource_text[casted_resource->file_size] = '\0';
                casted_resource->text = resource_text;
            }

            resources.push_back(resource);
            resource_path_map[canon_path] = resource;
            return ResourceManagerResult_OK;
            
        }

        static ResourceManagerResult unload_all_resources();
        static ResourceManagerResult unload_resource_by_path(const std::filesystem::path &path);
        static ResourceManagerResult unload_resource_by_alias(const std::string &alias);

    private:
        static inline std::vector<IResourceDescriptor*> resources;
        static inline std::unordered_map<std::filesystem::path, IResourceDescriptor*> resource_path_map;
        static inline std::unordered_map<std::string, IResourceDescriptor*> resource_alias_map;

};

