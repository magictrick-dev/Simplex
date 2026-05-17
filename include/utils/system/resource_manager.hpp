#pragma once
#include <string>
#include <filesystem>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <atomic>
#include <utility>
#include <queue>
#include <semaphore>

class ResourceInterface
{
    public:
        inline ResourceInterface(ResourceType resource_type) : type(resource_type) { }
        virtual inline ~ResourceInterface() = default;

        virtual void prepare() = 0;
        virtual void load() = 0;
        virtual void unload() = 0;
        virtual void unprepare() = 0;
        virtual void destroy() = 0;

        inline ResourceType get_resource_type() const { return this->type; }

    private:
        ResourceType type;

};

enum ResourceManagerResult
{
    ResourceManagerResult_OK,
    ResourceManagerResult_FileNotFound,
    ResourceManagerResult_FileNotAvailable,
    ResourceManagerResult_ResourceNotPrepared,
    ResourceManagerResult_ResourceNotFound,
    ResourceManagerResult_ResourceExists,
    ResourceManagerResult_HandleInvalid,
    ResourceManagerResult_HandleStale,
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

    inline ResourceHandle() = default;
    inline ResourceHandle(int32_t value) : handle(value) { }
    inline ResourceHandle(uint64_t value) : handle(value) { }
    inline ResourceHandle& operator=(int value) { this->handle = static_cast<uint64_t>(value); return *this; }
    inline ResourceHandle& operator=(uint64_t value) { this->handle = value; return *this; }
    inline bool operator==(const ResourceHandle &other) const { return this->handle == other.handle; }
    inline bool operator==(const uint64_t &other) const { return this->handle == other; }

};

enum ResourceState : uint32_t
{
    ResourceState_Unloaded      = 0,
    ResourceState_Unprepared    = 1,
    ResourceState_Prepared      = 2,
    ResourceState_Loading       = 3,
    ResourceState_Ready         = 4,
    ResourceState_Unloading     = 5,
};

enum ResourceType
{
    ResourceType_Invalid,
    ResourceType_BinaryFile,
    ResourceType_TextFile,
    ResourceType_ImageFile,
};

class ResourceManager
{

    public:
        static inline ResourceManager& Get() { static ResourceManager manager { }; return manager; }

        ResourceManagerResult register_binary_file_resource(const std::filesystem::path &path);
        ResourceManagerResult register_text_file_resource(const std::filesystem::path &path);
        ResourceManagerResult register_image_file_resource(const std::filesystem::path &path);

        ResourceManagerResult is_file_resource_registered(const std::filesystem::path &path) const;

        ResourceManagerResult get_file_resource_handle(const std::filesystem::path &path) const;
        ResourceManagerResult get_resource_state(const ResourceHandle &handle) const;

        ResourceManagerResult prepare(const ResourceHandle &handle);
        ResourceManagerResult load(const ResourceHandle &handle);
        ResourceManagerResult unload(const ResourceHandle &handle);
        ResourceManagerResult remove(const ResourceHandle &handle);
        ResourceManagerResult wait(const ResourceHandle &handle);

    private:
        struct
        {

            std::shared_mutex mutex;
            std::vector<ResourceInterface*> resources;
            std::vector<ResourceHandle> handles;
            std::vector<std::binary_semaphore> signals;
            std::vector<std::atomic<ResourceState>> states;
            std::queue<ResourceHandle> dead_handles;

            std::unordered_map<std::filesystem::path, size_t> file_mapping;

        } resources;

        struct
        {
            bool should_exit = false;

            std::mutex queue_mutex;
            std::condition_variable queue_condition;
            std::vector<std::thread> threads;
            std::queue<ResourceHandle> jobs;

        } thread_pool;

    private:
                    ResourceManager();
        virtual    ~ResourceManager();

        void        thread_pool_start();
        void        thread_pool_stop();
        void        thread_pool_enqueue(const ResourceHandle& handle);
        void        thread_pool_runtime();
        bool        thread_pool_fetch_job(ResourceHandle &handle);

};