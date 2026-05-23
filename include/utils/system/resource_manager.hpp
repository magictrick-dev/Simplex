#pragma once
#include <utils/defs.hpp>
#include <filesystem>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <atomic>
#include <queue>
#include <vector>
#include <thread>
#include <condition_variable>

// NOTE(Chris): States are intentionally minimal. "Preparedness" is implied on
//              successful registration: register_*_resource fetches metadata
//              syncronously (file existence, size, format header) and either
//              succeeds (state becomes Registered) or fails.
//
//              Registered: Metadata known, payload not in memory. Initial state
//                          after registration, and the terminal state after a
//                          successful unload.
//              Loading:    A worker is currently fetching the payload. wait()
//                          parks here lock-free via std::atomic::wait.
//              Ready:      Payload in memory; fetch_resource_view returns valid
//                          data.
//              Unloading:  A worker is currently freeing the payload. wait()
//                          parks here lock-free via std::atomic::wait.
enum ResourceState : uint32_t
{
    ResourceState_Registered    = 0,
    ResourceState_Loading       = 1,
    ResourceState_Ready         = 2,
    ResourceState_Unloading     = 3,
};

enum ResourceType
{
    ResourceType_Invalid,
    ResourceType_BinaryFile,
    ResourceType_TextFile,
    ResourceType_ImageFile,
};

class ResourceInterface
{
    public:
        inline ResourceInterface(ResourceType resource_type) : type(resource_type) { }
        virtual inline ~ResourceInterface() = default;

        virtual void load() = 0;
        virtual void unload() = 0;

        inline ResourceType get_resource_type() const { return this->type; }

    private:
        ResourceType type;

};

class BinaryFileResource : public ResourceInterface
{
    public:
                            BinaryFileResource(const std::filesystem::path &path, size_t size);
        virtual            ~BinaryFileResource();

        virtual void        load() override;
        virtual void        unload() override;

        inline const std::filesystem::path&     get_path() const { return this->path; }
        inline size_t                           get_size() const { return this->size; }
        inline const uint8_t*                   get_data() const { return this->data; }

    private:
        std::filesystem::path   path;
        size_t                  size;
        uint8_t                *data;

};

class TextFileResource : public ResourceInterface
{
    public:
                            TextFileResource(const std::filesystem::path &path, size_t length);
        virtual            ~TextFileResource();

        virtual void        load() override;
        virtual void        unload() override;

        inline const std::filesystem::path&     get_path() const { return this->path; }
        inline size_t                           get_length() const { return this->length; }
        inline const char*                      get_text() const { return this->text; }

    private:
        std::filesystem::path   path;
        size_t                  length; // bytes of payload (excludes trailing NUL)
        char                   *text;   // NUL-terminated; valid only in Ready state

};

class ImageRGBAResource : public ResourceInterface
{
    public:
                            ImageRGBAResource(const std::filesystem::path &path,
                                              int32_t width, int32_t height);
        virtual            ~ImageRGBAResource();

        virtual void        load() override;
        virtual void        unload() override;

        inline const std::filesystem::path&     get_path() const { return this->path; }
        inline int32_t                          get_width() const { return this->width; }
        inline int32_t                          get_height() const { return this->height; }
        inline const uint32_t*                  get_pixels() const { return this->pixels; }

    private:
        std::filesystem::path   path;
        int32_t                 width;
        int32_t                 height;
        uint32_t               *pixels; // packed RGBA, width*height entries; valid in Ready state

};

enum ResourceViewType
{
    ResourceViewType_Invalid,
    ResourceViewType_Binary,
    ResourceViewType_Text,
    ResourceViewType_RGBAImage,
};

struct BinaryResourceView
{
    size_t          size;
    const uint8_t  *data;
};

struct TextResourceView
{
    size_t          length;
    const char     *text;
};

struct ImageRGBAResourceView
{
    const uint32_t *pixels;
    int32_t         width;
    int32_t         height;
};

struct ResourceView
{

    ResourceViewType type;

    union
    {
        BinaryResourceView      binary_view;
        TextResourceView        text_view;
        ImageRGBAResourceView   image_rgba_view;
    };

};

enum ResourceManagerResult
{
    ResourceManagerResult_OK,
    ResourceManagerResult_FileNotFound,
    ResourceManagerResult_FileNotAvailable,
    ResourceManagerResult_ResourceNotReady,
    ResourceManagerResult_ResourceBusy,
    ResourceManagerResult_ResourceNotFound,
    ResourceManagerResult_ResourceExists,
    ResourceManagerResult_HandleInvalid,
    ResourceManagerResult_HandleStale,
    ResourceManagerResult_CapacityExceeded,
};

enum ResourceJobType
{
    ResourceJobType_Invalid,
    ResourceJobType_Load,
    ResourceJobType_Unload,
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

    inline ResourceHandle() : handle(0) { }
    inline ResourceHandle(int32_t value) : handle(value) { }
    inline ResourceHandle(uint64_t value) : handle(value) { }
    inline ResourceHandle& operator=(int value) { this->handle = static_cast<uint64_t>(value); return *this; }
    inline ResourceHandle& operator=(uint64_t value) { this->handle = value; return *this; }
    inline bool operator==(const ResourceHandle &other) const { return this->handle == other.handle; }
    inline bool operator==(const uint64_t &other) const { return this->handle == other; }
    inline bool operator!=(const ResourceHandle &other) const { return this->handle != other.handle; }

};

struct ResourceSlot
{
    // NOTE(Chris): `handle` is written once on slot allocation and not mutated
    //              until the slot is recycled (not yet implemented). When reuse
    //              is added, this must be promoted to an atomic so concurrent
    //              readers see a consistent identifier/generation pair.
    ResourceHandle              handle;
    ResourceInterface          *impl;
    std::atomic<ResourceState>  state;
};

struct ResourceJob
{
    ResourceJobType type;
    ResourceHandle  handle;
};

struct ResourceThreadpool
{

    std::atomic<bool>           should_exit;
    std::mutex                  queue_mutex;
    std::condition_variable     queue_condition;
    std::vector<std::thread>    threads;
    std::queue<ResourceJob>     jobs;

};

class ResourceManager
{

    public:
        static inline ResourceManager& Get() { static ResourceManager manager { }; return manager; }

        // NOTE(Chris): Registration is syncronous. The file must exist; metadata
        //              (size, etc.) is gathered before the handle is published.
        //              On success, the slot's state is Registered.
        ResourceManagerResult   register_binary_file_resource(const std::filesystem::path &path,
                                                              ResourceHandle *handle_out = NULL);
        ResourceManagerResult   register_text_file_resource(const std::filesystem::path &path,
                                                            ResourceHandle *handle_out = NULL);
        ResourceManagerResult   register_image_file_resource(const std::filesystem::path &path,
                                                             ResourceHandle *handle_out = NULL);

        ResourceManagerResult   is_file_resource_registered(const std::filesystem::path &path,
                                                            bool *registered_out) const;
        ResourceManagerResult   get_file_resource_handle(const std::filesystem::path &path,
                                                         ResourceHandle *handle_out) const;

        // NOTE(Chris): load/unload enqueue async work and return immediately.
        //              wait blocks lock-free (std::atomic::wait) until the slot
        //              settles out of Loading/Unloading. Callers should inspect
        //              the final state with the is_* queries if they need to
        //              know whether the resource is Ready vs Registered.
        ResourceManagerResult   load(const ResourceHandle &handle);
        ResourceManagerResult   unload(const ResourceHandle &handle);
        ResourceManagerResult   wait(const ResourceHandle &handle);
        ResourceManagerResult   remove(const ResourceHandle &handle);

        // NOTE(Chris): Synchronous load. Bypasses the threadpool entirely - the
        //              load() call runs inline on the caller's thread, so there
        //              is no queue dependency and no priority inversion if the
        //              pool is backed up with async work. If another thread is
        //              already loading the same handle (worker or another sync
        //              caller) this parks on the state via atomic::wait until
        //              they finish; if the resource is mid-Unloading it waits
        //              and then loads. Returns OK once the resource is Ready.
        ResourceManagerResult   load_synchronous(const ResourceHandle &handle);

        // NOTE(Chris): Symmetric to load_synchronous. Runs unload() inline. If
        //              the resource is mid-Loading, waits for it before tearing
        //              down. Returns OK once the resource is Registered.
        ResourceManagerResult   unload_synchronous(const ResourceHandle &handle);

        // NOTE(Chris): Lock-free snapshot queries. A `true` answer is a snapshot;
        //              the slot's state may change immediately after this returns.
        //              These silently return false for invalid or stale handles
        //              so they can be used in conditional checks without error
        //              handling. Use load/wait/fetch when you need a guarantee.
        bool                    is_registered(const ResourceHandle &handle) const;
        bool                    is_loading(const ResourceHandle &handle) const;
        bool                    is_ready(const ResourceHandle &handle) const;
        bool                    is_unloading(const ResourceHandle &handle) const;

        // NOTE(Chris): Caller must ensure the resource will not be unloaded for
        //              the duration the view is used. Pointers in the view alias
        //              memory owned by the resource and become dangling on unload.
        ResourceManagerResult   fetch_resource_view(const ResourceHandle &handle,
                                                    ResourceView *view) const;

    private:

        // NOTE(Chris): Fixed-capacity slot table. Slot pointers are heap allocated
        //              and have stable addresses for the lifetime of the manager,
        //              so readers can lock-free index into `slots[identifier]`
        //              with acquire ordering. Writers (register/remove) serialize
        //              on `registration_mutex` to allocate/recycle slots and to
        //              update the file_mapping index.
        static constexpr size_t MAX_RESOURCES = 4096;

        std::atomic<ResourceSlot*>  slots[MAX_RESOURCES];
        std::atomic<uint32_t>       slot_count;
        std::mutex                  registration_mutex;
        std::queue<uint32_t>        discarded_identifiers;
        std::unordered_map<std::filesystem::path, uint32_t> file_mapping;

        ResourceThreadpool          thread_pool;

    private:
                        ResourceManager();
        virtual        ~ResourceManager();

        ResourceSlot*   find_slot(const ResourceHandle &handle) const;

        void            thread_pool_start();
        void            thread_pool_stop();
        void            thread_pool_enqueue(ResourceJobType type, const ResourceHandle &handle);
        void            thread_pool_runtime();
        bool            thread_pool_fetch_job(ResourceJob &job);

};
