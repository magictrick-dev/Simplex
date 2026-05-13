#include <atomic>

#include <utils/defs.hpp>
#include <utils/system/memory_alloc.hpp>

#if defined(_WIN32)
#   include <malloc.h>
#endif

static std::atomic<size_t> memory_allocated = 0;
static std::atomic<size_t> memory_released = 0;
static std::atomic<size_t> memory_commit = 0;

size_t 
simplex_memory_get_allocations_total()
{
    return memory_allocated;
}

size_t 
simplex_memory_get_releases_total()
{
    return memory_released;
}

size_t 
simplex_memory_get_live()
{
    return memory_commit;
}

bool 
simplex_memory_is_balanced()
{
    return memory_allocated == memory_released;
}

void*   
simplex_internal_memory_alloc(size_t size, size_t alignment)
{

    SIMPLEX_ASSERT(alignment >= sizeof(AllocationDescriptor));
    SIMPLEX_ASSERT(alignment > 0 && (alignment & (alignment - 1)) == 0);

    size_t required_size = sizeof(AllocationDescriptor) + size;
    size_t aligned_size = ((required_size + alignment - 1) / alignment) * alignment;
    size_t forward_offset = alignment - sizeof(AllocationDescriptor);

#   if defined(_WIN32)
        void *result = _aligned_malloc(aligned_size, alignment);
#   elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
        void *result = std::aligned_alloc(alignment, aligned_size);
#   else
#       error "simplex_internal_memory_alloc: no aligned allocator defined for this platform."
#   endif

    SIMPLEX_ASSERT(result != NULL);

    AllocationDescriptor *descriptor = (AllocationDescriptor*)((uint8_t*)result + forward_offset);
    descriptor->memory_base = result;
    descriptor->memory_alignment = alignment;
    descriptor->memory_request_size = size;
    descriptor->memory_allocation_size = aligned_size;

    memory_allocated += aligned_size;
    memory_commit += aligned_size;

    void *user_ptr = descriptor + 1;
    return user_ptr;

}

void    
simplex_internal_memory_free(void* pointer)
{

    AllocationDescriptor *descriptor = (AllocationDescriptor*)pointer - 1;
    memory_released += descriptor->memory_allocation_size;
    memory_commit -= descriptor->memory_allocation_size;

    void *buffer_start = descriptor->memory_base;
#   if defined(_WIN32)
        _aligned_free(buffer_start);
#   elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
        std::free(buffer_start);
#   else
#       error "simplex_internal_memory_free: no aligned deallocator defined for this platform."
#   endif

}

AllocationDescriptor 
simplex_internal_memory_description(void *pointer)
{

#   if defined(SIMPLEX_ENABLE_MEMTRACKING) && SIMPLEX_ENABLE_MEMTRACKING != 0
        AllocationDescriptor *descriptor = (AllocationDescriptor*)pointer - 1;
        return *descriptor;
#   else
        return { };
#   endif

}

