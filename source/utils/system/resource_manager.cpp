#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <utility>

#include <utils/defs.hpp>
#include <utils/system/resource_manager.hpp>
#include <utils/system/memory_alloc.hpp>

enum ResourceType
{
    ResourceType_Invalid,
    ResourceType_BinaryFile,
    ResourceType_TextFile,
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
        inline size_t get_resource_size() const { return std::filesystem::file_size(this->canonical_path); }

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

        virtual inline bool is_ready() override 
        { 
            const char *file_path = this->get_resource_path().c_str();
            FILE *file_handle = fopen(file_path, "rb");
            if (file_handle == NULL) return false;
            fclose(file_handle);
            return true;
        }

        virtual inline bool is_available() override 
        { 
            const bool result = (this->buffer != NULL);
            return result;
        }

        virtual inline void load() override 
        { 

            size_t file_size = this->get_resource_size();

            const char *file_path = this->get_resource_path().c_str();
            FILE *file_handle = fopen(file_path, "rb");
            if (file_handle == NULL) return;

            void *file_buffer = simplex_memory_alloc(file_size);
            size_t read_size = fread(file_buffer, 1, file_size, file_handle);
            if (read_size != file_size)
            {
                simplex_memory_free(file_buffer);
            }
            else
            {
                buffer = file_buffer;
                size = file_size;
            }

            fclose(file_handle);

        }

        virtual inline void discard() override 
        { 
            simplex_memory_free(this->buffer);
            this->size = 0;
        }

        inline size_t binary_file_size() const { return this->size; }
        inline void* binary_file_buffer() const { return this->buffer; }

    private:
        void *buffer    = NULL;
        size_t size     = 0;

};

class TextFileResource : public FileResourceInterface
{
    public:
        inline TextFileResource() : FileResourceInterface(ResourceType_BinaryFile) { }
        inline TextFileResource(std::filesystem::path input) 
            : FileResourceInterface(ResourceType_TextFile)
        { 
            this->canonical_path = std::filesystem::weakly_canonical(input);
        }

        virtual inline bool is_ready() override 
        { 
            const char *file_path = this->get_resource_path().c_str();
            FILE *file_handle = fopen(file_path, "rb");
            if (file_handle == NULL) return false;
            fclose(file_handle);
            return true;
        }

        virtual inline bool is_available() override 
        { 
            const bool result = (this->buffer != NULL);
            return result;
        }

        virtual inline void load() override 
        { 

            size_t file_size = this->get_resource_size();

            const char *file_path = this->get_resource_path().c_str();
            FILE *file_handle = fopen(file_path, "rb");
            if (file_handle == NULL) return;

            void *file_buffer = simplex_memory_alloc(file_size+1);
            size_t read_size = fread(file_buffer, 1, file_size, file_handle);
            if (read_size != file_size)
            {
                simplex_memory_free(file_buffer);
            }
            else
            {
                ((char*)buffer)[file_size] = '\0';
                buffer = file_buffer;
                size = file_size;
            }

            fclose(file_handle);

        }

        virtual inline void discard() override 
        { 
            simplex_memory_free(this->buffer);
            this->size = 0;
        }

        inline size_t text_file_size() const { return this->size; }
        inline const char* c_str() const { return (char*)this->buffer; }

    private:
        void *buffer    = NULL;
        size_t size     = 0;

};

enum ResourceState : uint32_t
{
    ResourceState_Unloaded      = 0,
    ResourceState_Loading       = 1,
    ResourceState_Ready         = 2,
    ResourceState_Unloading     = 3,
};

struct ResourceManagerState
{

    static inline std::shared_mutex mtx;

    static inline std::vector<ResourceInterface*> resources;
    static inline std::vector<ResourceHandle> handles;
    static inline std::vector<std::atomic<ResourceState>> states;
    
};

static std::shared_mutex resource_mtx;

static std::vector<ResourceInterface*> resources;
static std::vector<ResourceHandle> handles;

using FileResourceDescription = std::pair<ResourceHandle, FileResourceInterface*>;
static std::unordered_map<std::filesystem::path, FileResourceDescription> registered_files;

template <typename T, typename... Args> inline static T*
CreateResource(ResourceHandle &handle, Args&&... args)
{

    // NOTE(Chris): Thread synchronization must occur externally, no sense over-generating
    //              locking code in a template if we don't need to.
    T* resource = simplex_memory_new<T>(std::forward<Args>(args)...);

    uint32_t current_identifier = static_cast<uint32_t>(resources.size());

    handle = {
        .identifier = current_identifier,
        .generation = 0,
    };

    resources.emplace_back(resource);
    handles.emplace_back(handle);

    return resource;

}

ResourceManagerResult ResourceManager::
IsFileResourceRegistered(std::filesystem::path input)
{

    std::filesystem::path canonical_path = std::filesystem::weakly_canonical(input);

    std::shared_lock lock(resource_mtx);
    const bool result = (registered_files.find(input) != registered_files.end());
    return (result ? ResourceManagerResult_ResourceExists : ResourceManagerResult_ResourceNotFound);

}

ResourceManagerResult ResourceManager::
RegisterBinaryFile(std::filesystem::path input, ResourceHandle *handle)
{

    std::filesystem::path canonical_path = std::filesystem::weakly_canonical(input);
    if (!std::filesystem::exists(canonical_path)) return ResourceManagerResult_FileNotFound;
    if (IsFileResourceRegistered(canonical_path)) return ResourceManagerResult_OK;

    FileResourceInterface *resource_interface = NULL;
    ResourceHandle resource_handle = {};

    {
        std::unique_lock lock(resource_mtx);
        resource_interface = CreateResource<BinaryFileResource>(resource_handle, canonical_path);
        registered_files[canonical_path] = { resource_handle, resource_interface };
    }

    if (handle != NULL) *handle = resource_handle;
    return ResourceManagerResult_OK;

}

ResourceManagerResult ResourceManager::
RegisterTextFile(std::filesystem::path input, ResourceHandle *handle)
{

    std::filesystem::path canonical_path = std::filesystem::weakly_canonical(input);
    if (!std::filesystem::exists(canonical_path)) return ResourceManagerResult_FileNotFound;
    if (IsFileResourceRegistered(canonical_path)) return ResourceManagerResult_OK;

    FileResourceInterface *resource_interface = NULL;
    ResourceHandle resource_handle = {};

    {
        std::unique_lock lock(resource_mtx);
        resource_interface = CreateResource<TextFileResource>(resource_handle, canonical_path);
        registered_files[canonical_path] = { resource_handle, resource_interface };
    }

    if (handle != NULL) *handle = resource_handle;
    return ResourceManagerResult_OK;

}