#pragma once
#include <utility>

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

void*   simplex_virtual_allocate(size_t size);
void    simplex_virtual_free(size_t size);
size_t  simplex_virtual_page_size();
size_t  simplex_virtual_allocation_size(void* virtual_allocated_buffer);

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
simplex_memory_new(Args&&... args)
{

    void* raw_buffer = simplex_memory_alloc(sizeof(T));
    T* result = new (raw_buffer) T(std::forward<Args>(args)...);
    return result;

}

template <typename T> inline void
simplex_memory_delete(T *buffer)
{

    buffer->~T();
    simplex_memory_free(buffer);

}

struct MemoryArena
{
    void  *buffer;
    size_t size;
    size_t offset_bottom;
    size_t offset_top;
};

typedef size_t arena_stamp_t;

bool            simplex_allocate_memory_arena(MemoryArena *arena, size_t request_size);
void            simplex_deallocate_memory_arena(MemoryArena *arena);
void*           simplex_memory_arena_push_bottom(MemoryArena *arena, size_t size);
void*           simplex_memory_arena_push_top(MemoryArena *arena, size_t size);
void            simplex_memory_arena_pop_bottom(MemoryArena *arena, size_t size);
void            simplex_memory_arena_pop_top(MemoryArena *arena, size_t size);
arena_stamp_t   simplex_memory_arena_stamp_bottom(MemoryArena *arena);
arena_stamp_t   simplex_memory_arena_stamp_top(MemoryArena *arena);
void            simplex_memory_arena_restore_bottom(MemoryArena *arena, arena_stamp_t stamp);
void            simplex_memory_arena_restore_top(MemoryArena *arena, arena_stamp_t stamp);
size_t          simplex_memory_arena_get_commit(MemoryArena *arena);

class scoped_arena_stamp
{
    public:
        inline scoped_arena_stamp(MemoryArena *arena) 
        {
            this->arena         = arena;
            this->stamp_bottom  = simplex_memory_arena_stamp_bottom(arena);
            this->stamp_top     = simplex_memory_arena_stamp_top(arena);
        };

        inline ~scoped_arena_stamp()
        {
            simplex_memory_arena_restore_bottom(this->arena, this->stamp_bottom);
            simplex_memory_arena_restore_top(this->arena, this->stamp_top);
        }
    
    private:
        MemoryArena *arena;
        arena_stamp_t stamp_bottom;
        arena_stamp_t stamp_top;
};

size_t simplex_memory_get_allocations_total();
size_t simplex_memory_get_releases_total();
size_t simplex_memory_get_live();
size_t simplex_memory_get_peak();
size_t simplex_memory_get_virtual_allocations_total();
size_t simplex_memory_get_virtual_releases_total();
size_t simplex_memory_get_virtual_live();
void simplex_memory_reset_peak();
bool simplex_memory_is_balanced();