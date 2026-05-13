#pragma once

struct AllocationDescriptor 
{
    void       *memory_base;
    size_t      memory_request_size;
    size_t      memory_allocation_size;
    size_t      memory_alignment;
};

// NOTE(Chris): Do not use this functions directly, call the macro version as it
//              directly alias to the proper version.
void*   simplex_internal_memory_alloc(size_t size, size_t alignment);
void    simplex_internal_memory_free(void* pointer);
AllocationDescriptor simplex_internal_memory_description(void *pointer);

#define SIMPLEX_ENABLE_MEMTRACKING 1
#if defined(SIMPLEX_ENABLE_MEMTRACKING) && SIMPLEX_ENABLE_MEMTRACKING != 0
#   define simplex_memory_alloc(size) simplex_internal_memory_alloc(size, 32)
#   define simplex_memory_alloc_aligned(size, alignment) simplex_internal_memory_alloc(size, alignment)
#   define simplex_memory_free(ptr) simplex_internal_memory_free(ptr)
#   define simplex_memory_descriptor(ptr) simplex_internal_memory_description(ptr)
#else
#   define simplex_memory_alloc(size) malloc(size)
#   define simplex_memory_alloc_aligned(size, alignment) malloc(size)
#   define simplex_memory_free(ptr) free(ptr)
#   define simplex_memory_descriptor(ptr) simplex_internal_memory_description(ptr)
#endif

template <typename T, typename... Args> inline T*
simplex_memory_new(Args... args)
{

    void* raw_buffer = simplex_memory_alloc(sizeof(T));
    T* result = new (raw_buffer) T(args...);
    return result;

}

template <typename T> inline void
simplex_memory_delete(T *buffer)
{

    buffer->~T();
    simplex_memory_free(buffer);

}

size_t simplex_memory_get_allocations_total();
size_t simplex_memory_get_releases_total();
size_t simplex_memory_get_live();
bool simplex_memory_is_balanced();