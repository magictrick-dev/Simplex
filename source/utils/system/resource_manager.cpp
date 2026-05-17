#include <utils/defs.hpp>
#include <utils/system/resource_manager.hpp>
#include <utils/system/memory_alloc.hpp>

ResourceManager::
ResourceManager()
{
    this->thread_pool_start();
}

ResourceManager::
~ResourceManager()
{
    this->thread_pool_stop();
}

void ResourceManager::
thread_pool_start()
{

    // NOTE(Chris): If someone is using a fossil to run this software, we enforce a
    //              minimum of 4 threads and let the operating system schedule out the
    //              work. For the most part, the threads are just doing I/O work.
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

}

void ResourceManager::
thread_pool_runtime()
{

    while (true)
    {

        ResourceHandle handle = {};
        if (!thread_pool_fetch_job(handle)) return;

        // TODO(Chris): We have the job, now we load it.

    }

}

bool ResourceManager::
thread_pool_fetch_job(ResourceHandle &handle)
{

    std::unique_lock lock(this->thread_pool.queue_mutex);
    this->thread_pool.queue_condition.wait(lock, [this] {
        return (this->thread_pool.jobs.empty() || this->thread_pool.should_exit);
    });

    if (this->thread_pool.should_exit == true) return false;

    handle = this->thread_pool.jobs.front();
    this->thread_pool.jobs.pop();

    return true;

}

void ResourceManager::
thread_pool_enqueue(const ResourceHandle& handle)
{

}

ResourceManagerResult ResourceManager::
register_binary_file_resource(const std::filesystem::path &path)
{

}

ResourceManagerResult ResourceManager::
register_text_file_resource(const std::filesystem::path &path)
{

}

ResourceManagerResult ResourceManager::
register_image_file_resource(const std::filesystem::path &path)
{

}

ResourceManagerResult ResourceManager::
is_file_resource_registered(const std::filesystem::path &path) const
{

}

ResourceManagerResult ResourceManager::
get_file_resource_handle(const std::filesystem::path &path) const
{

}

ResourceManagerResult ResourceManager::
get_resource_state(const ResourceHandle &handle) const
{

}

ResourceManagerResult ResourceManager::
wait(const ResourceHandle &handle)
{

}

ResourceManagerResult ResourceManager::
prepare(const ResourceHandle &handle)
{

}

ResourceManagerResult ResourceManager::
load(const ResourceHandle &handle)
{

}

ResourceManagerResult ResourceManager::
unload(const ResourceHandle &handle)
{

}

ResourceManagerResult ResourceManager::
remove(const ResourceHandle &handle)
{

}