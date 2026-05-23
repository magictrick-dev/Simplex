#include <utils/defs.hpp>
#include <utils/system/resource_manager.hpp>
#include <utils/system/memory_alloc.hpp>

#include <cstdio>
#include <cstring>
#include <system_error>

// Route stb_image through our tracked allocator so image payloads show up in
// the memory subsystem. realloc is emulated (alloc + copy + free) because
// simplex_memory_alloc has no realloc primitive.
static inline void*
simplex_stbi_malloc(size_t size)
{
    return simplex_memory_alloc(size);
}

static inline void
simplex_stbi_free(void *pointer)
{
    if (pointer == NULL) return;
    simplex_memory_free(pointer);
}

static inline void*
simplex_stbi_realloc(void *pointer, size_t new_size)
{

    if (pointer == NULL) return simplex_memory_alloc(new_size);
    if (new_size == 0)
    {
        simplex_memory_free(pointer);
        return NULL;
    }

    AllocationDescriptor descriptor = simplex_memory_descriptor(pointer);
    void *new_pointer = simplex_memory_alloc(new_size);
    size_t copy_bytes = (descriptor.memory_request_size < new_size)
                      ? descriptor.memory_request_size : new_size;
    std::memcpy(new_pointer, pointer, copy_bytes);
    simplex_memory_free(pointer);
    return new_pointer;

}

#define STBI_MALLOC(sz)         simplex_stbi_malloc(sz)
#define STBI_REALLOC(p, sz)     simplex_stbi_realloc((p), (sz))
#define STBI_FREE(p)            simplex_stbi_free(p)
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

BinaryFileResource::
BinaryFileResource(const std::filesystem::path &path, size_t size)
    : ResourceInterface(ResourceType_BinaryFile),
      path(path),
      size(size),
      data(NULL)
{

}

BinaryFileResource::
~BinaryFileResource()
{

    if (this->data != NULL)
    {
        simplex_memory_free(this->data);
        this->data = NULL;
    }

}

void BinaryFileResource::
load()
{

    SIMPLEX_ASSERT(this->data == NULL);

    if (this->size == 0)
    {
        this->data = (uint8_t*)simplex_memory_alloc(1);
        return;
    }

    FILE *file = NULL;
#   if defined(_WIN32)
        fopen_s(&file, this->path.string().c_str(), "rb");
#   else
        file = std::fopen(this->path.string().c_str(), "rb");
#   endif
    SIMPLEX_CHECK_PTR(file);

    this->data = (uint8_t*)simplex_memory_alloc(this->size);
    SIMPLEX_CHECK_PTR(this->data);

    size_t read = std::fread(this->data, 1, this->size, file);
    SIMPLEX_ASSERT(read == this->size);
    (void)read;

    std::fclose(file);

}

void BinaryFileResource::
unload()
{

    if (this->data != NULL)
    {
        simplex_memory_free(this->data);
        this->data = NULL;
    }

}

TextFileResource::
TextFileResource(const std::filesystem::path &path, size_t length)
    : ResourceInterface(ResourceType_TextFile),
      path(path),
      length(length),
      text(NULL)
{

}

TextFileResource::
~TextFileResource()
{

    if (this->text != NULL)
    {
        simplex_memory_free(this->text);
        this->text = NULL;
    }

}

void TextFileResource::
load()
{

    SIMPLEX_ASSERT(this->text == NULL);

    // Always allocate length+1 so we can stamp a trailing NUL even when the
    // file is empty - callers that treat get_text() as a C string always
    // dereference safely.
    this->text = (char*)simplex_memory_alloc(this->length + 1);
    SIMPLEX_CHECK_PTR(this->text);

    if (this->length == 0)
    {
        this->text[0] = '\0';
        return;
    }

    FILE *file = NULL;
#   if defined(_WIN32)
        fopen_s(&file, this->path.string().c_str(), "rb");
#   else
        file = std::fopen(this->path.string().c_str(), "rb");
#   endif
    SIMPLEX_CHECK_PTR(file);

    size_t read = std::fread(this->text, 1, this->length, file);
    SIMPLEX_ASSERT(read == this->length);
    (void)read;

    this->text[this->length] = '\0';

    std::fclose(file);

}

void TextFileResource::
unload()
{

    if (this->text != NULL)
    {
        simplex_memory_free(this->text);
        this->text = NULL;
    }

}

ImageRGBAResource::
ImageRGBAResource(const std::filesystem::path &path, int32_t width, int32_t height)
    : ResourceInterface(ResourceType_ImageFile),
      path(path),
      width(width),
      height(height),
      pixels(NULL)
{

}

ImageRGBAResource::
~ImageRGBAResource()
{

    if (this->pixels != NULL)
    {
        simplex_memory_free(this->pixels);
        this->pixels = NULL;
    }

}

void ImageRGBAResource::
load()
{

    SIMPLEX_ASSERT(this->pixels == NULL);

    int decoded_width    = 0;
    int decoded_height   = 0;
    int decoded_channels = 0;

    // NOTE(Chris): req_comp = 4 forces stb_image to return packed RGBA8 regardless
    //              of the source format. The returned buffer is sized as
    //              width * height * 4 bytes; we reinterpret it as uint32_t* per
    //              the ImageRGBAResourceView contract.
    stbi_uc *decoded = stbi_load(this->path.string().c_str(),
                                 &decoded_width, &decoded_height,
                                 &decoded_channels, 4);
    SIMPLEX_CHECK_PTR(decoded);
    SIMPLEX_ASSERT(decoded_width  == this->width);
    SIMPLEX_ASSERT(decoded_height == this->height);

    this->pixels = reinterpret_cast<uint32_t*>(decoded);

}

void ImageRGBAResource::
unload()
{

    if (this->pixels != NULL)
    {
        // stbi_image_free routes through STBI_FREE -> simplex_memory_free.
        stbi_image_free(this->pixels);
        this->pixels = NULL;
    }

}

ResourceManager::
ResourceManager()
    : slot_count(0)
{

    for (size_t i = 0; i < MAX_RESOURCES; ++i)
        this->slots[i].store(NULL, std::memory_order_relaxed);

    this->thread_pool.should_exit.store(false, std::memory_order_relaxed);
    this->thread_pool_start();

}

ResourceManager::
~ResourceManager()
{

    this->thread_pool_stop();

    // NOTE(Chris): Drain all remaining slots. We are single-threaded here.
    for (size_t i = 0; i < MAX_RESOURCES; ++i)
    {

        ResourceSlot *slot = this->slots[i].load(std::memory_order_relaxed);
        if (slot == NULL) continue;

        if (slot->impl != NULL)
        {
            // Make sure the payload is released; impl destructor also handles
            // this, but call unload explicitly to keep the state machine honest.
            ResourceState state = slot->state.load(std::memory_order_relaxed);
            if (state == ResourceState_Ready) slot->impl->unload();
            simplex_memory_delete(slot->impl);
            slot->impl = NULL;
        }

        simplex_memory_delete(slot);
        this->slots[i].store(NULL, std::memory_order_relaxed);

    }

}

void ResourceManager::
thread_pool_start()
{

    // NOTE(Chris): If someone is using a fossil to run this software, we enforce
    //              a minimum of 4 threads and let the operating system schedule
    //              out the work. For the most part the threads are doing I/O.
    uint32_t thread_count = std::thread::hardware_concurrency();
    if (thread_count == 0 || thread_count < 4) thread_count = 4;

    for (uint32_t i = 0; i < thread_count; ++i)
    {
        this->thread_pool.threads.emplace_back(std::thread(
            &ResourceManager::thread_pool_runtime, this));
    }

}

void ResourceManager::
thread_pool_stop()
{

    {
        std::unique_lock<std::mutex> lock(this->thread_pool.queue_mutex);
        this->thread_pool.should_exit.store(true, std::memory_order_release);
    }

    this->thread_pool.queue_condition.notify_all();
    for (auto &thread : this->thread_pool.threads) thread.join();
    this->thread_pool.threads.clear();

}

void ResourceManager::
thread_pool_enqueue(ResourceJobType type, const ResourceHandle &handle)
{

    {
        std::unique_lock<std::mutex> lock(this->thread_pool.queue_mutex);
        this->thread_pool.jobs.push({ type, handle });
    }
    this->thread_pool.queue_condition.notify_one();

}

bool ResourceManager::
thread_pool_fetch_job(ResourceJob &job)
{

    std::unique_lock<std::mutex> lock(this->thread_pool.queue_mutex);
    this->thread_pool.queue_condition.wait(lock, [this] {
        return (!this->thread_pool.jobs.empty() ||
                 this->thread_pool.should_exit.load(std::memory_order_acquire));
    });

    if (this->thread_pool.should_exit.load(std::memory_order_acquire) &&
        this->thread_pool.jobs.empty()) return false;

    job = this->thread_pool.jobs.front();
    this->thread_pool.jobs.pop();
    return true;

}

void ResourceManager::
thread_pool_runtime()
{

    while (true)
    {

        ResourceJob job = { ResourceJobType_Invalid, ResourceHandle((uint64_t)0) };
        if (!this->thread_pool_fetch_job(job)) break;

        ResourceSlot *slot = this->find_slot(job.handle);
        if (slot == NULL) continue;

        // NOTE(Chris): Generation mismatch means the slot was recycled while the
        //              job was queued. Drop it on the floor.
        if (slot->handle != job.handle) continue;

        switch (job.type)
        {

            case ResourceJobType_Load:
            {
                slot->impl->load();
                slot->state.store(ResourceState_Ready, std::memory_order_release);
                slot->state.notify_all();
            } break;

            case ResourceJobType_Unload:
            {
                slot->impl->unload();
                slot->state.store(ResourceState_Registered, std::memory_order_release);
                slot->state.notify_all();
            } break;

            default:
            {
                SIMPLEX_NO_REACH("thread_pool_runtime: unknown job type");
            } break;

        }

    }

}

ResourceSlot* ResourceManager::
find_slot(const ResourceHandle &handle) const
{

    if (handle.identifier >= MAX_RESOURCES) return NULL;
    return this->slots[handle.identifier].load(std::memory_order_acquire);

}

static ResourceManagerResult
publish_resource_locked(std::atomic<ResourceSlot*> *slot_table,
                        std::atomic<uint32_t> *slot_count_counter,
                        std::queue<uint32_t> *discarded,
                        std::unordered_map<std::filesystem::path, uint32_t> *file_mapping,
                        size_t max_resources,
                        const std::filesystem::path &canonical,
                        ResourceInterface *impl,
                        ResourceHandle *handle_out)
{

    uint32_t identifier = 0;
    ResourceSlot *slot  = NULL;

    if (!discarded->empty())
    {
        identifier = discarded->front();
        discarded->pop();
        slot = slot_table[identifier].load(std::memory_order_relaxed);
        SIMPLEX_CHECK_PTR(slot);
    }
    else
    {
        identifier = slot_count_counter->fetch_add(1, std::memory_order_relaxed);
        if (identifier >= max_resources)
        {
            slot_count_counter->fetch_sub(1, std::memory_order_relaxed);
            simplex_memory_delete(impl);
            return ResourceManagerResult_CapacityExceeded;
        }
        slot = simplex_memory_new<ResourceSlot>();
        slot->handle = ResourceHandle((uint64_t)0);
        slot->handle.identifier = identifier;
        slot->handle.generation = 1;
    }

    slot->impl = impl;
    slot->state.store(ResourceState_Registered, std::memory_order_relaxed);

    // Release-store publishes the fully-initialised slot to lock-free readers.
    slot_table[identifier].store(slot, std::memory_order_release);
    file_mapping->emplace(canonical, identifier);

    if (handle_out != NULL) *handle_out = slot->handle;
    return ResourceManagerResult_OK;

}

ResourceManagerResult ResourceManager::
register_binary_file_resource(const std::filesystem::path &path, ResourceHandle *handle_out)
{

    std::error_code ec;

    if (!std::filesystem::exists(path, ec) || ec)
        return ResourceManagerResult_FileNotFound;

    std::filesystem::path canonical = std::filesystem::canonical(path, ec);
    if (ec) return ResourceManagerResult_FileNotAvailable;

    uintmax_t file_size = std::filesystem::file_size(canonical, ec);
    if (ec) return ResourceManagerResult_FileNotAvailable;

    std::unique_lock<std::mutex> lock(this->registration_mutex);

    auto existing = this->file_mapping.find(canonical);
    if (existing != this->file_mapping.end())
    {
        ResourceSlot *slot = this->slots[existing->second].load(std::memory_order_acquire);
        if (handle_out != NULL && slot != NULL) *handle_out = slot->handle;
        return ResourceManagerResult_ResourceExists;
    }

    BinaryFileResource *impl = simplex_memory_new<BinaryFileResource>(
        canonical, (size_t)file_size);

    return publish_resource_locked(this->slots, &this->slot_count,
                                   &this->discarded_identifiers, &this->file_mapping,
                                   MAX_RESOURCES, canonical, impl, handle_out);

}

ResourceManagerResult ResourceManager::
register_text_file_resource(const std::filesystem::path &path, ResourceHandle *handle_out)
{

    std::error_code ec;

    if (!std::filesystem::exists(path, ec) || ec)
        return ResourceManagerResult_FileNotFound;

    std::filesystem::path canonical = std::filesystem::canonical(path, ec);
    if (ec) return ResourceManagerResult_FileNotAvailable;

    uintmax_t file_size = std::filesystem::file_size(canonical, ec);
    if (ec) return ResourceManagerResult_FileNotAvailable;

    std::unique_lock<std::mutex> lock(this->registration_mutex);

    auto existing = this->file_mapping.find(canonical);
    if (existing != this->file_mapping.end())
    {
        ResourceSlot *slot = this->slots[existing->second].load(std::memory_order_acquire);
        if (handle_out != NULL && slot != NULL) *handle_out = slot->handle;
        return ResourceManagerResult_ResourceExists;
    }

    TextFileResource *impl = simplex_memory_new<TextFileResource>(
        canonical, (size_t)file_size);

    return publish_resource_locked(this->slots, &this->slot_count,
                                   &this->discarded_identifiers, &this->file_mapping,
                                   MAX_RESOURCES, canonical, impl, handle_out);

}

ResourceManagerResult ResourceManager::
register_image_file_resource(const std::filesystem::path &path, ResourceHandle *handle_out)
{

    std::error_code ec;

    if (!std::filesystem::exists(path, ec) || ec)
        return ResourceManagerResult_FileNotFound;

    std::filesystem::path canonical = std::filesystem::canonical(path, ec);
    if (ec) return ResourceManagerResult_FileNotAvailable;

    // NOTE(Chris): stbi_info reads only the image header; cheap enough to do
    //              at registration time so the caller knows width/height before
    //              committing to a load and we can reject malformed files early.
    int header_width  = 0;
    int header_height = 0;
    int header_comp   = 0;
    if (!stbi_info(canonical.string().c_str(), &header_width, &header_height, &header_comp))
        return ResourceManagerResult_FileNotAvailable;

    std::unique_lock<std::mutex> lock(this->registration_mutex);

    auto existing = this->file_mapping.find(canonical);
    if (existing != this->file_mapping.end())
    {
        ResourceSlot *slot = this->slots[existing->second].load(std::memory_order_acquire);
        if (handle_out != NULL && slot != NULL) *handle_out = slot->handle;
        return ResourceManagerResult_ResourceExists;
    }

    ImageRGBAResource *impl = simplex_memory_new<ImageRGBAResource>(
        canonical, (int32_t)header_width, (int32_t)header_height);

    return publish_resource_locked(this->slots, &this->slot_count,
                                   &this->discarded_identifiers, &this->file_mapping,
                                   MAX_RESOURCES, canonical, impl, handle_out);

}

ResourceManagerResult ResourceManager::
is_file_resource_registered(const std::filesystem::path &path, bool *registered_out) const
{

    SIMPLEX_CHECK_PTR(registered_out);

    std::error_code ec;
    std::filesystem::path canonical = std::filesystem::weakly_canonical(path, ec);
    if (ec) canonical = path;

    // NOTE(Chris): registration_mutex is non-const; const_cast is the pragmatic
    //              workaround until we promote file_mapping to its own lock-free
    //              structure. Reads are rare enough that this is fine.
    std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(this->registration_mutex));
    *registered_out = (this->file_mapping.find(canonical) != this->file_mapping.end());
    return ResourceManagerResult_OK;

}

ResourceManagerResult ResourceManager::
get_file_resource_handle(const std::filesystem::path &path, ResourceHandle *handle_out) const
{

    SIMPLEX_CHECK_PTR(handle_out);

    std::error_code ec;
    std::filesystem::path canonical = std::filesystem::weakly_canonical(path, ec);
    if (ec) canonical = path;

    std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(this->registration_mutex));
    auto it = this->file_mapping.find(canonical);
    if (it == this->file_mapping.end()) return ResourceManagerResult_ResourceNotFound;

    ResourceSlot *slot = this->slots[it->second].load(std::memory_order_acquire);
    if (slot == NULL) return ResourceManagerResult_ResourceNotFound;

    *handle_out = slot->handle;
    return ResourceManagerResult_OK;

}

ResourceManagerResult ResourceManager::
load(const ResourceHandle &handle)
{

    ResourceSlot *slot = this->find_slot(handle);
    if (slot == NULL) return ResourceManagerResult_HandleInvalid;
    if (slot->handle != handle) return ResourceManagerResult_HandleStale;

    ResourceState expected = ResourceState_Registered;
    if (slot->state.compare_exchange_strong(expected, ResourceState_Loading,
                                            std::memory_order_acq_rel,
                                            std::memory_order_acquire))
    {
        this->thread_pool_enqueue(ResourceJobType_Load, handle);
        return ResourceManagerResult_OK;
    }

    // CAS failed - `expected` now holds the observed state.
    if (expected == ResourceState_Loading)   return ResourceManagerResult_OK;
    if (expected == ResourceState_Ready)     return ResourceManagerResult_OK;
    return ResourceManagerResult_ResourceBusy; // Unloading

}

ResourceManagerResult ResourceManager::
load_synchronous(const ResourceHandle &handle)
{

    ResourceSlot *slot = this->find_slot(handle);
    if (slot == NULL) return ResourceManagerResult_HandleInvalid;
    if (slot->handle != handle) return ResourceManagerResult_HandleStale;

    // NOTE(Chris): We bypass the threadpool. The CAS loop handles three races:
    //                Registered -> Loading: we own the load, do it inline.
    //                Loading             : someone else owns it; park and retry.
    //                Unloading           : wait for teardown, then retry.
    //                Ready               : already done; no-op.
    while (true)
    {

        ResourceState expected = ResourceState_Registered;
        if (slot->state.compare_exchange_strong(expected, ResourceState_Loading,
                                                std::memory_order_acq_rel,
                                                std::memory_order_acquire))
        {
            slot->impl->load();
            slot->state.store(ResourceState_Ready, std::memory_order_release);
            slot->state.notify_all();
            return ResourceManagerResult_OK;
        }

        // CAS failed - `expected` reflects what we actually observed.
        if (expected == ResourceState_Ready) return ResourceManagerResult_OK;

        if (expected == ResourceState_Loading ||
            expected == ResourceState_Unloading)
        {
            // Park lock-free until the state moves; then re-evaluate. If we
            // saw Loading we'll likely see Ready and return; if we saw
            // Unloading we'll likely see Registered and take the CAS.
            slot->state.wait(expected, std::memory_order_acquire);
            continue;
        }

        // Defensive - the enum is closed, but if it ever grows we want to know.
        SIMPLEX_NO_REACH("load_synchronous: unexpected state");
        return ResourceManagerResult_ResourceBusy;

    }

}

ResourceManagerResult ResourceManager::
unload(const ResourceHandle &handle)
{

    ResourceSlot *slot = this->find_slot(handle);
    if (slot == NULL) return ResourceManagerResult_HandleInvalid;
    if (slot->handle != handle) return ResourceManagerResult_HandleStale;

    ResourceState expected = ResourceState_Ready;
    if (slot->state.compare_exchange_strong(expected, ResourceState_Unloading,
                                            std::memory_order_acq_rel,
                                            std::memory_order_acquire))
    {
        this->thread_pool_enqueue(ResourceJobType_Unload, handle);
        return ResourceManagerResult_OK;
    }

    if (expected == ResourceState_Registered) return ResourceManagerResult_OK;
    if (expected == ResourceState_Unloading)  return ResourceManagerResult_OK;
    return ResourceManagerResult_ResourceBusy; // Loading

}

ResourceManagerResult ResourceManager::
unload_synchronous(const ResourceHandle &handle)
{

    ResourceSlot *slot = this->find_slot(handle);
    if (slot == NULL) return ResourceManagerResult_HandleInvalid;
    if (slot->handle != handle) return ResourceManagerResult_HandleStale;

    // NOTE(Chris): Mirror of load_synchronous. The CAS targets Ready->Unloading
    //              instead of Registered->Loading; everything else is symmetric.
    //                Ready      : we own the unload, do it inline.
    //                Unloading  : someone else owns it; park and retry.
    //                Loading    : wait for the load to finish, then retry.
    //                Registered : already unloaded; no-op.
    while (true)
    {

        ResourceState expected = ResourceState_Ready;
        if (slot->state.compare_exchange_strong(expected, ResourceState_Unloading,
                                                std::memory_order_acq_rel,
                                                std::memory_order_acquire))
        {
            slot->impl->unload();
            slot->state.store(ResourceState_Registered, std::memory_order_release);
            slot->state.notify_all();
            return ResourceManagerResult_OK;
        }

        if (expected == ResourceState_Registered) return ResourceManagerResult_OK;

        if (expected == ResourceState_Loading ||
            expected == ResourceState_Unloading)
        {
            slot->state.wait(expected, std::memory_order_acquire);
            continue;
        }

        SIMPLEX_NO_REACH("unload_synchronous: unexpected state");
        return ResourceManagerResult_ResourceBusy;

    }

}

ResourceManagerResult ResourceManager::
wait(const ResourceHandle &handle)
{

    ResourceSlot *slot = this->find_slot(handle);
    if (slot == NULL) return ResourceManagerResult_HandleInvalid;
    if (slot->handle != handle) return ResourceManagerResult_HandleStale;

    // NOTE(Chris): Lock-free park. Loops only if the value changed to another
    //              transitional state (Loading <-> Unloading), which is rare.
    while (true)
    {

        ResourceState cur = slot->state.load(std::memory_order_acquire);
        if (cur != ResourceState_Loading && cur != ResourceState_Unloading)
            return ResourceManagerResult_OK;

        slot->state.wait(cur, std::memory_order_acquire);

    }

}

ResourceManagerResult ResourceManager::
remove(const ResourceHandle &handle)
{

    ResourceSlot *slot = this->find_slot(handle);
    if (slot == NULL) return ResourceManagerResult_HandleInvalid;
    if (slot->handle != handle) return ResourceManagerResult_HandleStale;

    std::unique_lock<std::mutex> lock(this->registration_mutex);

    // Re-check state under the lock; only resources sitting at rest in the
    // Registered state can be removed. Callers must unload + wait first.
    ResourceState state = slot->state.load(std::memory_order_acquire);
    if (state != ResourceState_Registered) return ResourceManagerResult_ResourceBusy;

    // Erase the file mapping using the path the impl knows about - this avoids
    // path-canonicalisation drift from the caller's lookup path.
    if (slot->impl != NULL)
    {
        switch (slot->impl->get_resource_type())
        {
            case ResourceType_BinaryFile:
                this->file_mapping.erase(
                    static_cast<BinaryFileResource*>(slot->impl)->get_path());
                break;
            case ResourceType_TextFile:
                this->file_mapping.erase(
                    static_cast<TextFileResource*>(slot->impl)->get_path());
                break;
            case ResourceType_ImageFile:
                this->file_mapping.erase(
                    static_cast<ImageRGBAResource*>(slot->impl)->get_path());
                break;
            default:
                SIMPLEX_NO_REACH("remove: unknown resource type");
                break;
        }
    }

    if (slot->impl != NULL)
    {
        simplex_memory_delete(slot->impl);
        slot->impl = NULL;
    }

    // Bump generation and recycle the identifier. The slot itself stays
    // allocated - subsequent registrations pick up the same address with a new
    // generation, which atomically invalidates any cached handle.
    slot->handle.generation += 1;
    this->discarded_identifiers.push(handle.identifier);

    return ResourceManagerResult_OK;

}

bool ResourceManager::
is_registered(const ResourceHandle &handle) const
{

    ResourceSlot *slot = this->find_slot(handle);
    if (slot == NULL || slot->handle != handle) return false;
    return slot->state.load(std::memory_order_acquire) == ResourceState_Registered;

}

bool ResourceManager::
is_loading(const ResourceHandle &handle) const
{

    ResourceSlot *slot = this->find_slot(handle);
    if (slot == NULL || slot->handle != handle) return false;
    return slot->state.load(std::memory_order_acquire) == ResourceState_Loading;

}

bool ResourceManager::
is_ready(const ResourceHandle &handle) const
{

    ResourceSlot *slot = this->find_slot(handle);
    if (slot == NULL || slot->handle != handle) return false;
    return slot->state.load(std::memory_order_acquire) == ResourceState_Ready;

}

bool ResourceManager::
is_unloading(const ResourceHandle &handle) const
{

    ResourceSlot *slot = this->find_slot(handle);
    if (slot == NULL || slot->handle != handle) return false;
    return slot->state.load(std::memory_order_acquire) == ResourceState_Unloading;

}

ResourceManagerResult ResourceManager::
fetch_resource_view(const ResourceHandle &handle, ResourceView *view) const
{

    SIMPLEX_CHECK_PTR(view);

    ResourceSlot *slot = this->find_slot(handle);
    if (slot == NULL) return ResourceManagerResult_HandleInvalid;
    if (slot->handle != handle) return ResourceManagerResult_HandleStale;

    if (slot->state.load(std::memory_order_acquire) != ResourceState_Ready)
        return ResourceManagerResult_ResourceNotReady;

    SIMPLEX_CHECK_PTR(slot->impl);

    switch (slot->impl->get_resource_type())
    {

        case ResourceType_BinaryFile:
        {
            BinaryFileResource *binary = static_cast<BinaryFileResource*>(slot->impl);
            view->type = ResourceViewType_Binary;
            view->binary_view.size = binary->get_size();
            view->binary_view.data = binary->get_data();
            return ResourceManagerResult_OK;
        }

        case ResourceType_TextFile:
        {
            TextFileResource *text = static_cast<TextFileResource*>(slot->impl);
            view->type = ResourceViewType_Text;
            view->text_view.length = text->get_length();
            view->text_view.text   = text->get_text();
            return ResourceManagerResult_OK;
        }

        case ResourceType_ImageFile:
        {
            ImageRGBAResource *image = static_cast<ImageRGBAResource*>(slot->impl);
            view->type = ResourceViewType_RGBAImage;
            view->image_rgba_view.pixels = image->get_pixels();
            view->image_rgba_view.width  = image->get_width();
            view->image_rgba_view.height = image->get_height();
            return ResourceManagerResult_OK;
        }

        default:
        {
            view->type = ResourceViewType_Invalid;
            return ResourceManagerResult_ResourceNotFound;
        }

    }

}

// ------------------------------------------------------------------------------------------------- //
// Tests                                                                                             //
// ------------------------------------------------------------------------------------------------- //

#include <utils/test_registry.hpp>
#if defined(SIMPLEX_ENABLE_TESTS) && SIMPLEX_ENABLE_TESTS == 1
#include <fstream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

struct EmptyCase
{
};

struct BinaryFileCase
{
    size_t  payload_bytes;
    int     waiter_count;
};

// Temp file helper. Writes `bytes` bytes (seeded deterministic stream) to a
// uniquely named file in the temp directory and returns the path. Caller is
// responsible for removing it.
static std::filesystem::path
make_temp_binary_file(size_t bytes, uint8_t seed = 0xA5)
{

    std::filesystem::path dir = std::filesystem::temp_directory_path();
    static std::atomic<uint32_t> counter{0};
    uint32_t id = counter.fetch_add(1, std::memory_order_relaxed);

    char name[64];
    std::snprintf(name, sizeof(name), "simplex_rm_test_%u_%llu.bin",
                  id, (unsigned long long)bytes);
    std::filesystem::path path = dir / name;

    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    std::vector<uint8_t> buffer(bytes);
    uint8_t value = seed;
    for (size_t i = 0; i < bytes; ++i)
    {
        buffer[i] = value;
        value = (uint8_t)(value * 31 + 7);
    }
    if (bytes > 0)
        stream.write((const char*)buffer.data(), (std::streamsize)bytes);
    stream.close();

    return path;

}

static bool
buffer_matches_seed(const uint8_t *data, size_t size, uint8_t seed)
{

    uint8_t value = seed;
    for (size_t i = 0; i < size; ++i)
    {
        if (data[i] != value) return false;
        value = (uint8_t)(value * 31 + 7);
    }
    return true;

}

static bool
rm_register_missing_file_fails(const EmptyCase &)
{

    ResourceManager &manager = ResourceManager::Get();

    std::filesystem::path bogus =
        std::filesystem::temp_directory_path() / "simplex_does_not_exist_12345.bin";

    ResourceHandle handle = (uint64_t)0;
    ResourceManagerResult r = manager.register_binary_file_resource(bogus, &handle);
    return r == ResourceManagerResult_FileNotFound;

}

static bool
rm_register_succeeds(const BinaryFileCase &test_case)
{

    ResourceManager &manager = ResourceManager::Get();
    std::filesystem::path path = make_temp_binary_file(test_case.payload_bytes);

    ResourceHandle handle = (uint64_t)0;
    ResourceManagerResult r = manager.register_binary_file_resource(path, &handle);

    bool ok = (r == ResourceManagerResult_OK)
           && manager.is_registered(handle)
           && !manager.is_ready(handle);

    manager.remove(handle);
    std::filesystem::remove(path);
    return ok;

}

static bool
rm_register_duplicate_returns_existing(const BinaryFileCase &test_case)
{

    ResourceManager &manager = ResourceManager::Get();
    std::filesystem::path path = make_temp_binary_file(test_case.payload_bytes);

    ResourceHandle first  = (uint64_t)0;
    ResourceHandle second = (uint64_t)0;
    manager.register_binary_file_resource(path, &first);
    ResourceManagerResult r = manager.register_binary_file_resource(path, &second);

    bool ok = (r == ResourceManagerResult_ResourceExists)
           && (first == second);

    manager.remove(first);
    std::filesystem::remove(path);
    return ok;

}

static bool
rm_load_wait_fetch(const BinaryFileCase &test_case)
{

    ResourceManager &manager = ResourceManager::Get();
    const uint8_t seed = 0xC3;
    std::filesystem::path path = make_temp_binary_file(test_case.payload_bytes, seed);

    ResourceHandle handle = (uint64_t)0;
    manager.register_binary_file_resource(path, &handle);

    bool ok = (manager.load(handle) == ResourceManagerResult_OK);
    ok = ok && (manager.wait(handle) == ResourceManagerResult_OK);
    ok = ok && manager.is_ready(handle);

    ResourceView view = {};
    ok = ok && (manager.fetch_resource_view(handle, &view) == ResourceManagerResult_OK);
    ok = ok && (view.type == ResourceViewType_Binary);
    ok = ok && (view.binary_view.size == test_case.payload_bytes);
    if (ok && test_case.payload_bytes > 0)
        ok = buffer_matches_seed(view.binary_view.data, view.binary_view.size, seed);

    // Cycle through unload so we exercise the full state machine before remove,
    // which requires Registered.
    ok = ok && (manager.unload(handle) == ResourceManagerResult_OK);
    ok = ok && (manager.wait(handle) == ResourceManagerResult_OK);
    ok = ok && manager.is_registered(handle);

    manager.remove(handle);
    std::filesystem::remove(path);
    return ok;

}

// Kick off a load and spin up N concurrent waiters that race the worker.
// Each waiter either parks on Loading (lock-free via atomic::wait) or
// short-circuits on Ready - both must end with is_ready() == true.
static bool
rm_concurrent_waiters_all_wake(const BinaryFileCase &test_case)
{

    ResourceManager &manager = ResourceManager::Get();
    std::filesystem::path path = make_temp_binary_file(test_case.payload_bytes);

    ResourceHandle handle = (uint64_t)0;
    manager.register_binary_file_resource(path, &handle);

    // NOTE(Chris): load() must be called before spawning waiters. wait() only
    //              parks on transitional states (Loading/Unloading); a waiter
    //              observing Registered would return immediately and report a
    //              spurious not-ready result.
    manager.load(handle);

    std::atomic<int> ready_seen{0};
    std::atomic<int> wait_ok{0};

    std::vector<std::thread> waiters;
    waiters.reserve((size_t)test_case.waiter_count);

    for (int i = 0; i < test_case.waiter_count; ++i)
    {
        waiters.emplace_back([&] {
            if (manager.wait(handle) == ResourceManagerResult_OK)
                wait_ok.fetch_add(1, std::memory_order_relaxed);
            if (manager.is_ready(handle))
                ready_seen.fetch_add(1, std::memory_order_relaxed);
        });
    }

    for (auto &t : waiters) t.join();

    // Ensure the load worker is fully settled before tearing down; otherwise a
    // racing unload() could collide with the in-flight load (CAS would fail
    // with ResourceBusy and leak the payload).
    manager.wait(handle);

    bool ok = (wait_ok.load()    == test_case.waiter_count)
           && (ready_seen.load() == test_case.waiter_count);

    manager.unload(handle);
    manager.wait(handle);
    manager.remove(handle);
    std::filesystem::remove(path);
    return ok;

}

// Calling wait() after the load already finished must return immediately
// (the fast path - no parking).
static bool
rm_wait_after_ready_short_circuits(const BinaryFileCase &test_case)
{

    ResourceManager &manager = ResourceManager::Get();
    std::filesystem::path path = make_temp_binary_file(test_case.payload_bytes);

    ResourceHandle handle = (uint64_t)0;
    manager.register_binary_file_resource(path, &handle);

    manager.load(handle);
    manager.wait(handle);

    auto start = std::chrono::high_resolution_clock::now();
    ResourceManagerResult r = manager.wait(handle);
    auto elapsed = std::chrono::high_resolution_clock::now() - start;

    bool ok = (r == ResourceManagerResult_OK)
           && (std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() < 1000)
           && manager.is_ready(handle);

    manager.unload(handle);
    manager.wait(handle);
    manager.remove(handle);
    std::filesystem::remove(path);
    return ok;

}

static bool
rm_invalid_handle_rejected(const EmptyCase &)
{

    ResourceManager &manager = ResourceManager::Get();

    ResourceHandle bogus = (uint64_t)0;
    bogus.identifier = 9999;
    bogus.generation = 42;

    ResourceView view = {};
    ResourceManagerResult fetch_result = manager.fetch_resource_view(bogus, &view);
    ResourceManagerResult wait_result  = manager.wait(bogus);

    bool fetch_rejected = (fetch_result == ResourceManagerResult_HandleInvalid)
                       || (fetch_result == ResourceManagerResult_HandleStale);
    bool wait_rejected  = (wait_result  == ResourceManagerResult_HandleInvalid)
                       || (wait_result  == ResourceManagerResult_HandleStale);

    return fetch_rejected
        && wait_rejected
        && !manager.is_ready(bogus)
        && !manager.is_loading(bogus);

}

static bool
rm_stale_handle_rejected(const BinaryFileCase &test_case)
{

    ResourceManager &manager = ResourceManager::Get();
    std::filesystem::path path = make_temp_binary_file(test_case.payload_bytes);

    ResourceHandle handle = (uint64_t)0;
    manager.register_binary_file_resource(path, &handle);
    manager.remove(handle);

    // After remove(), the slot bumps its generation. The stale handle should be
    // rejected as Stale (slot pointer still exists, generation differs).
    ResourceView view = {};
    ResourceManagerResult r = manager.fetch_resource_view(handle, &view);

    bool ok = (r == ResourceManagerResult_HandleStale)
           && !manager.is_ready(handle)
           && !manager.is_registered(handle);

    std::filesystem::remove(path);
    return ok;

}

// Basic sync path - load_synchronous must transition Registered -> Ready
// before returning, with no wait() needed by the caller.
static bool
rm_sync_load_basic(const BinaryFileCase &test_case)
{

    ResourceManager &manager = ResourceManager::Get();
    const uint8_t seed = 0x77;
    std::filesystem::path path = make_temp_binary_file(test_case.payload_bytes, seed);

    ResourceHandle handle = (uint64_t)0;
    manager.register_binary_file_resource(path, &handle);

    bool ok = (manager.load_synchronous(handle) == ResourceManagerResult_OK);
    // No wait() call - the resource must already be Ready on return.
    ok = ok && manager.is_ready(handle);

    ResourceView view = {};
    ok = ok && (manager.fetch_resource_view(handle, &view) == ResourceManagerResult_OK);
    ok = ok && (view.binary_view.size == test_case.payload_bytes);
    if (ok && test_case.payload_bytes > 0)
        ok = buffer_matches_seed(view.binary_view.data, view.binary_view.size, seed);

    manager.unload(handle);
    manager.wait(handle);
    manager.remove(handle);
    std::filesystem::remove(path);
    return ok;

}

// Calling load_synchronous when the resource is already Ready must be a no-op.
static bool
rm_sync_load_idempotent(const BinaryFileCase &test_case)
{

    ResourceManager &manager = ResourceManager::Get();
    std::filesystem::path path = make_temp_binary_file(test_case.payload_bytes);

    ResourceHandle handle = (uint64_t)0;
    manager.register_binary_file_resource(path, &handle);

    bool ok = (manager.load_synchronous(handle) == ResourceManagerResult_OK);
    ok = ok && (manager.load_synchronous(handle) == ResourceManagerResult_OK);
    ok = ok && (manager.load_synchronous(handle) == ResourceManagerResult_OK);
    ok = ok && manager.is_ready(handle);

    manager.unload(handle);
    manager.wait(handle);
    manager.remove(handle);
    std::filesystem::remove(path);
    return ok;

}

// Race: kick off async load(), then call load_synchronous on the same handle.
// Either the worker won the CAS (sync parks on Loading until notify), or the
// sync caller won (worker job ends up a no-op when it observes Ready/!=
// Registered). Both paths must end with the resource Ready and correctly
// decoded.
static bool
rm_sync_load_races_with_async(const BinaryFileCase &test_case)
{

    ResourceManager &manager = ResourceManager::Get();
    const uint8_t seed = 0xB9;
    std::filesystem::path path = make_temp_binary_file(test_case.payload_bytes, seed);

    ResourceHandle handle = (uint64_t)0;
    manager.register_binary_file_resource(path, &handle);

    manager.load(handle);                                  // queue async load
    bool ok = (manager.load_synchronous(handle)            // race with worker
                == ResourceManagerResult_OK);
    ok = ok && manager.is_ready(handle);

    ResourceView view = {};
    ok = ok && (manager.fetch_resource_view(handle, &view) == ResourceManagerResult_OK);
    ok = ok && buffer_matches_seed(view.binary_view.data, view.binary_view.size, seed);

    // Drain any in-flight worker before teardown so we don't leak across tests.
    manager.wait(handle);
    manager.unload(handle);
    manager.wait(handle);
    manager.remove(handle);
    std::filesystem::remove(path);
    return ok;

}

// Sync unload after sync load - both phases must complete inline, with the
// final state Registered before either call returns.
static bool
rm_sync_unload_basic(const BinaryFileCase &test_case)
{

    ResourceManager &manager = ResourceManager::Get();
    std::filesystem::path path = make_temp_binary_file(test_case.payload_bytes);

    ResourceHandle handle = (uint64_t)0;
    manager.register_binary_file_resource(path, &handle);

    bool ok = (manager.load_synchronous(handle) == ResourceManagerResult_OK);
    ok = ok && manager.is_ready(handle);

    ok = ok && (manager.unload_synchronous(handle) == ResourceManagerResult_OK);
    // No wait() call - the resource must already be Registered on return.
    ok = ok && manager.is_registered(handle);
    ok = ok && !manager.is_ready(handle);

    manager.remove(handle);
    std::filesystem::remove(path);
    return ok;

}

// Calling unload_synchronous on an already-unloaded resource must be a no-op.
static bool
rm_sync_unload_idempotent(const BinaryFileCase &test_case)
{

    ResourceManager &manager = ResourceManager::Get();
    std::filesystem::path path = make_temp_binary_file(test_case.payload_bytes);

    ResourceHandle handle = (uint64_t)0;
    manager.register_binary_file_resource(path, &handle);

    // From freshly Registered - three sync unloads in a row should all OK.
    bool ok = (manager.unload_synchronous(handle) == ResourceManagerResult_OK);
    ok = ok && (manager.unload_synchronous(handle) == ResourceManagerResult_OK);
    ok = ok && (manager.unload_synchronous(handle) == ResourceManagerResult_OK);
    ok = ok && manager.is_registered(handle);

    // And again after a real load/unload cycle.
    manager.load_synchronous(handle);
    manager.unload_synchronous(handle);
    ok = ok && (manager.unload_synchronous(handle) == ResourceManagerResult_OK);
    ok = ok && manager.is_registered(handle);

    manager.remove(handle);
    std::filesystem::remove(path);
    return ok;

}

// Race: kick off async load(), then call unload_synchronous on the same
// handle. Sync unload sees Loading, parks on it, the worker finishes the load
// (state -> Ready), sync unload wakes and re-loops, CAS Ready -> Unloading
// wins, sync runs the unload inline. End state: Registered, no leaks.
static bool
rm_sync_unload_waits_for_async_load(const BinaryFileCase &test_case)
{

    ResourceManager &manager = ResourceManager::Get();
    std::filesystem::path path = make_temp_binary_file(test_case.payload_bytes);

    ResourceHandle handle = (uint64_t)0;
    manager.register_binary_file_resource(path, &handle);

    manager.load(handle);                                  // queue async load
    bool ok = (manager.unload_synchronous(handle)          // race with worker
                == ResourceManagerResult_OK);
    ok = ok && manager.is_registered(handle);
    ok = ok && !manager.is_ready(handle);

    manager.remove(handle);
    std::filesystem::remove(path);
    return ok;

}

struct TextFileCase
{
    const char *payload;
};

static std::filesystem::path
make_temp_text_file(const char *payload)
{

    std::filesystem::path dir = std::filesystem::temp_directory_path();
    static std::atomic<uint32_t> counter{0};
    uint32_t id = counter.fetch_add(1, std::memory_order_relaxed);

    char name[64];
    std::snprintf(name, sizeof(name), "simplex_rm_test_%u.txt", id);
    std::filesystem::path path = dir / name;

    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (payload != NULL && payload[0] != '\0')
        stream.write(payload, (std::streamsize)std::strlen(payload));
    stream.close();

    return path;

}

static bool
rm_text_load_wait_fetch(const TextFileCase &test_case)
{

    ResourceManager &manager = ResourceManager::Get();
    std::filesystem::path path = make_temp_text_file(test_case.payload);

    ResourceHandle handle = (uint64_t)0;
    ResourceManagerResult r = manager.register_text_file_resource(path, &handle);
    bool ok = (r == ResourceManagerResult_OK);

    ok = ok && (manager.load(handle) == ResourceManagerResult_OK);
    ok = ok && (manager.wait(handle) == ResourceManagerResult_OK);
    ok = ok && manager.is_ready(handle);

    ResourceView view = {};
    ok = ok && (manager.fetch_resource_view(handle, &view) == ResourceManagerResult_OK);
    ok = ok && (view.type == ResourceViewType_Text);

    size_t expected_length = (test_case.payload == NULL) ? 0 : std::strlen(test_case.payload);
    ok = ok && (view.text_view.length == expected_length);
    ok = ok && (view.text_view.text != NULL);
    // The NUL terminator must always be present so callers can treat text as a
    // C string regardless of length.
    ok = ok && (view.text_view.text[expected_length] == '\0');
    if (ok && expected_length > 0)
        ok = (std::memcmp(view.text_view.text, test_case.payload, expected_length) == 0);

    ok = ok && (manager.unload(handle) == ResourceManagerResult_OK);
    ok = ok && (manager.wait(handle) == ResourceManagerResult_OK);

    manager.remove(handle);
    std::filesystem::remove(path);
    return ok;

}

struct ImageCase
{
    int width;
    int height;
};

static inline void
write_le_u16(uint8_t *out, uint16_t value)
{
    out[0] = (uint8_t)(value & 0xFF);
    out[1] = (uint8_t)((value >> 8) & 0xFF);
}

static inline void
write_le_u32(uint8_t *out, uint32_t value)
{
    out[0] = (uint8_t)(value & 0xFF);
    out[1] = (uint8_t)((value >> 8) & 0xFF);
    out[2] = (uint8_t)((value >> 16) & 0xFF);
    out[3] = (uint8_t)((value >> 24) & 0xFF);
}

// Hand-rolled 2x2 24-bit BMP. stb_image will decode this with req_comp=4 and
// expand it to RGBA8 with alpha = 0xFF on every pixel.
//
// Pixel layout, top-down (top-left -> bottom-right):
//   Red(0,0)   Green(1,0)
//   Blue(0,1)  White(1,1)
//
// BMP stores bottom-up with BGR triplets and 4-byte-aligned rows.
static std::filesystem::path
make_temp_bmp_2x2()
{

    std::filesystem::path dir = std::filesystem::temp_directory_path();
    static std::atomic<uint32_t> counter{0};
    uint32_t id = counter.fetch_add(1, std::memory_order_relaxed);

    char name[64];
    std::snprintf(name, sizeof(name), "simplex_rm_test_%u.bmp", id);
    std::filesystem::path path = dir / name;

    constexpr int32_t  width        = 2;
    constexpr int32_t  height       = 2;
    constexpr uint32_t row_bytes    = 6;            // 2 px * 3 bytes
    constexpr uint32_t row_padded   = 8;            // align to 4
    constexpr uint32_t image_size   = row_padded * height;
    constexpr uint32_t data_offset  = 54;
    constexpr uint32_t file_size    = data_offset + image_size;

    uint8_t bmp[data_offset + image_size] = {};

    // BITMAPFILEHEADER
    bmp[0] = 'B';
    bmp[1] = 'M';
    write_le_u32(&bmp[2],  file_size);
    write_le_u16(&bmp[6],  0);
    write_le_u16(&bmp[8],  0);
    write_le_u32(&bmp[10], data_offset);

    // BITMAPINFOHEADER
    write_le_u32(&bmp[14], 40);              // header size
    write_le_u32(&bmp[18], (uint32_t)width);
    write_le_u32(&bmp[22], (uint32_t)height);
    write_le_u16(&bmp[26], 1);               // planes
    write_le_u16(&bmp[28], 24);              // bits per pixel
    write_le_u32(&bmp[30], 0);               // BI_RGB
    write_le_u32(&bmp[34], image_size);
    write_le_u32(&bmp[38], 0);
    write_le_u32(&bmp[42], 0);
    write_le_u32(&bmp[46], 0);
    write_le_u32(&bmp[50], 0);

    // Pixel data, bottom-up. Row 0 in the file is the BOTTOM row (y=1).
    uint8_t *row_bottom = &bmp[data_offset + 0 * row_padded];
    uint8_t *row_top    = &bmp[data_offset + 1 * row_padded];

    // Bottom row: Blue (0,1), White (1,1)
    row_bottom[0] = 0xFF; row_bottom[1] = 0x00; row_bottom[2] = 0x00; // Blue  BGR
    row_bottom[3] = 0xFF; row_bottom[4] = 0xFF; row_bottom[5] = 0xFF; // White
    // Top row: Red (0,0), Green (1,0)
    row_top[0]    = 0x00; row_top[1]    = 0x00; row_top[2]    = 0xFF; // Red
    row_top[3]    = 0x00; row_top[4]    = 0xFF; row_top[5]    = 0x00; // Green

    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write((const char*)bmp, sizeof(bmp));
    stream.close();

    return path;

}

static bool
rm_image_load_wait_fetch(const ImageCase &)
{

    ResourceManager &manager = ResourceManager::Get();
    std::filesystem::path path = make_temp_bmp_2x2();

    ResourceHandle handle = (uint64_t)0;
    ResourceManagerResult r = manager.register_image_file_resource(path, &handle);
    bool ok = (r == ResourceManagerResult_OK);

    ok = ok && (manager.load(handle) == ResourceManagerResult_OK);
    ok = ok && (manager.wait(handle) == ResourceManagerResult_OK);
    ok = ok && manager.is_ready(handle);

    ResourceView view = {};
    ok = ok && (manager.fetch_resource_view(handle, &view) == ResourceManagerResult_OK);
    ok = ok && (view.type == ResourceViewType_RGBAImage);
    ok = ok && (view.image_rgba_view.width  == 2);
    ok = ok && (view.image_rgba_view.height == 2);
    ok = ok && (view.image_rgba_view.pixels != NULL);

    // RGBA packed in memory order R,G,B,A. On little-endian uint32 this reads as
    // 0xAA'BB'GG'RR. stb forces alpha = 0xFF on a 24-bit BMP load.
    if (ok)
    {
        const uint32_t *p = view.image_rgba_view.pixels;
        const uint32_t expected_red   = 0xFF0000FFu;
        const uint32_t expected_green = 0xFF00FF00u;
        const uint32_t expected_blue  = 0xFFFF0000u;
        const uint32_t expected_white = 0xFFFFFFFFu;

        ok = ok && (p[0] == expected_red)
                && (p[1] == expected_green)
                && (p[2] == expected_blue)
                && (p[3] == expected_white);
    }

    ok = ok && (manager.unload(handle) == ResourceManagerResult_OK);
    ok = ok && (manager.wait(handle) == ResourceManagerResult_OK);

    manager.remove(handle);
    std::filesystem::remove(path);
    return ok;

}

static bool
rm_image_register_rejects_garbage(const EmptyCase &)
{

    ResourceManager &manager = ResourceManager::Get();

    // Build a temp file with bytes that aren't a valid image of any kind.
    std::filesystem::path path = make_temp_text_file("not an image at all, just plain bytes");

    ResourceHandle handle = (uint64_t)0;
    ResourceManagerResult r = manager.register_image_file_resource(path, &handle);

    bool ok = (r == ResourceManagerResult_FileNotAvailable);

    std::filesystem::remove(path);
    return ok;

}

SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "register: missing file fails",
    rm_register_missing_file_fails, EmptyCase);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "register: empty payload",
    rm_register_succeeds, BinaryFileCase, 0, 0);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "register: 1KB payload",
    rm_register_succeeds, BinaryFileCase, 1024, 0);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "register: duplicate returns existing",
    rm_register_duplicate_returns_existing, BinaryFileCase, 256, 0);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "load/wait/fetch: empty",
    rm_load_wait_fetch, BinaryFileCase, 0, 0);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "load/wait/fetch: 4KB",
    rm_load_wait_fetch, BinaryFileCase, 4096, 0);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "load/wait/fetch: 1MB",
    rm_load_wait_fetch, BinaryFileCase, 1024 * 1024, 0);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "wait: 8 concurrent waiters",
    rm_concurrent_waiters_all_wake, BinaryFileCase, 64 * 1024, 8);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "wait: 32 concurrent waiters",
    rm_concurrent_waiters_all_wake, BinaryFileCase, 64 * 1024, 32);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "wait: short-circuits on Ready",
    rm_wait_after_ready_short_circuits, BinaryFileCase, 4096, 0);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "handle: invalid rejected",
    rm_invalid_handle_rejected, EmptyCase);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "handle: stale rejected after remove",
    rm_stale_handle_rejected, BinaryFileCase, 128, 0);

SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "text: empty payload",
    rm_text_load_wait_fetch, TextFileCase, "");
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "text: short payload",
    rm_text_load_wait_fetch, TextFileCase, "hello, world");
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "text: multi-line payload",
    rm_text_load_wait_fetch, TextFileCase,
    "first line\nsecond line\nthird line with trailing newline\n");

SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "image: 2x2 BMP round-trip",
    rm_image_load_wait_fetch, ImageCase, 2, 2);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "image: garbage file rejected",
    rm_image_register_rejects_garbage, EmptyCase);

SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "sync load: basic",
    rm_sync_load_basic, BinaryFileCase, 4096, 0);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "sync load: empty payload",
    rm_sync_load_basic, BinaryFileCase, 0, 0);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "sync load: idempotent",
    rm_sync_load_idempotent, BinaryFileCase, 1024, 0);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "sync load: races with async",
    rm_sync_load_races_with_async, BinaryFileCase, 64 * 1024, 0);

SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "sync unload: basic",
    rm_sync_unload_basic, BinaryFileCase, 4096, 0);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "sync unload: idempotent",
    rm_sync_unload_idempotent, BinaryFileCase, 1024, 0);
SIMPLEX_REGISTER_GROUPED_TEST("System: resource_manager", "sync unload: waits for async load",
    rm_sync_unload_waits_for_async_load, BinaryFileCase, 64 * 1024, 0);

#endif // SIMPLEX_ENABLE_TESTS
