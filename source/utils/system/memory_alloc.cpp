#include <atomic>
#include <mutex>
#include <unordered_map>

#include <utils/defs.hpp>
#include <utils/system/memory_alloc.hpp>

#if defined(_WIN32)
#   include <malloc.h>
#   include <windows.h>
#endif

#if defined(__APPLE__) || defined(__unix__)
#   include <sys/mman.h>
#   include <unistd.h>
#endif

#if defined(__APPLE__)
#   include <mach/mach.h>
#   include <mach/mach_vm.h>
#endif

static std::atomic<size_t> memory_allocated         = 0;
static std::atomic<size_t> memory_released          = 0;
static std::atomic<size_t> memory_commit            = 0;
static std::atomic<size_t> memory_peak              = 0;
static std::atomic<size_t> virtual_memory_commit    = 0;
static std::atomic<size_t> virtual_memory_allocated = 0;
static std::atomic<size_t> virtual_memory_released  = 0;

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

size_t
simplex_memory_get_peak()
{
    return memory_peak;
}

void
simplex_memory_reset_peak()
{
    memory_peak = memory_commit.load();
}

bool
simplex_memory_is_balanced()
{
    return memory_allocated == memory_released;
}

size_t 
simplex_memory_get_virtual_allocations_total()
{
    return virtual_memory_allocated;
}

size_t 
simplex_memory_get_virtual_releases_total()
{
    return virtual_memory_released;
}

size_t 
simplex_memory_get_virtual_live()
{
    return virtual_memory_commit;
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
    size_t live = (memory_commit += aligned_size);

    size_t observed_peak = memory_peak.load();
    while (observed_peak < live && !memory_peak.compare_exchange_weak(observed_peak, live));

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

static std::mutex virtual_mapping_mutex;
static std::unordered_map<void*,size_t> virtual_allocation_sizes;

void*   
simplex_virtual_allocate(size_t size)
{

    SIMPLEX_ASSERT(size > 0);
    const size_t page_size = simplex_virtual_page_size();
    const size_t rounded_size = ((size + page_size - 1) / page_size) * page_size;

    void *result = NULL;

#   if defined(_WIN32)
        result = VirtualAlloc(nullptr, rounded_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#   elif defined(__APPLE__) || defined(__unix__)
        result = mmap(nullptr, rounded_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#   else
#       pragma error "Undefined virtual allocation for given platform."
#   endif

    if (result != NULL)
    {
        std::scoped_lock lock(virtual_mapping_mutex);
        virtual_allocation_sizes[result] = rounded_size;
        virtual_memory_allocated += rounded_size;
        virtual_memory_commit += rounded_size;
    }

    return result;

}

void    
simplex_virtual_free(void *buffer)
{

    SIMPLEX_CHECK_PTR(buffer);

    size_t allocation_size = 0;

    {
        std::scoped_lock lock(virtual_mapping_mutex);
        auto it = virtual_allocation_sizes.find(buffer);
        if (it == virtual_allocation_sizes.end()) return;
        allocation_size = it->second;
        virtual_allocation_sizes.erase(it);
    }

#   if defined(_WIN32)
        VirtualFree(buffer, 0, MEM_RELEASE);
#   elif defined(__APPLE__) || defined(__unix__)
        munmap(buffer, allocation_size);
#   else
#       pragma error "Undefined virtual free for given platform."
#   endif

    virtual_memory_released += allocation_size;
    virtual_memory_commit -= allocation_size;

}

size_t  
simplex_virtual_page_size()
{

#   if defined(_WIN32)
        SYSTEM_INFO system_info = {};
        GetSystemInfo(&system_info);
        return static_cast<size_t>(system_info.dwAllocationGranularity);
#   elif defined(__APPLE__) || defined(__unix__)
        const size_t page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));
        if (page_size == 0) return 4096;
        return page_size;
#   else
#       pragma error "Undefined virtual page size for given platform."
#   endif

}

size_t  
simplex_virtual_allocation_size(void* virtual_allocated_buffer)
{

    if (!virtual_allocated_buffer) return 0;

#   if defined(_WIN32)

        MEMORY_BASIC_INFORMATION info = {};
        if (VirtualQuery(virtual_allocated_buffer, &info, sizeof(info)) == 0)
        {
            return 0;
        }

        return static_cast<size_t>(info.RegionSize);

#   elif defined(__APPLE__)

        mach_vm_address_t address = reinterpret_cast<mach_vm_address_t>(virtual_allocated_buffer);
        mach_vm_size_t size = 0;

        vm_region_basic_info_data_64_t info = {};
        mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT_64;
        mach_port_t object_name = MACH_PORT_NULL;

        kern_return_t result = mach_vm_region(
            mach_task_self(),
            &address,
            &size,
            VM_REGION_BASIC_INFO_64,
            reinterpret_cast<vm_region_info_t>(&info),
            &info_count,
            &object_name
        );

        if (result != KERN_SUCCESS)
        {
            return 0;
        }

        return static_cast<size_t>(size);

#   elif defined(__linux__)

        FILE* maps = std::fopen("/proc/self/maps", "r");
        if (!maps) return 0;

        const uintptr_t pointer = reinterpret_cast<uintptr_t>(virtual_allocated_buffer);

        char line[512];
        while (std::fgets(line, sizeof(line), maps))
        {
            uintptr_t begin = 0;
            uintptr_t end = 0;

            if (std::sscanf(line, "%zx-%zx", &begin, &end) == 2)
            {
                if (pointer >= begin && pointer < end)
                {
                    std::fclose(maps);
                    return static_cast<size_t>(end - begin);
                }
            }
        }

        std::fclose(maps);
        return 0;

#   elif defined(__unix__)

        // Generic Unix has no portable OS query for mmap region size.
        // Fall back to the tracked size.
        std::lock_guard<std::mutex> lock(virtual_mapping_mutex);

        auto it = virtual_allocation_sizes.find(virtual_allocated_buffer);
        if (it == virtual_allocation_sizes.end()) return 0;

        return it->second;

#   else

        std::lock_guard<std::mutex> lock(virtual_mapping_mutex);

        auto it = virtual_allocation_sizes.find(virtual_allocated_buffer);
        if (it == virtual_allocation_sizes.end()) return 0;

        return it->second;

#   endif

}

bool            
simplex_allocate_memory_arena(MemoryArena *arena, size_t request_size)
{

    SIMPLEX_CHECK_PTR(arena);
    void *buffer = simplex_virtual_allocate(request_size);

    if (buffer == NULL)
    {
        return false;
    }

    arena->buffer           = buffer;
    arena->offset_bottom    = 0;
    arena->offset_top       = 0;
    arena->size             = simplex_virtual_allocation_size(buffer);
    SIMPLEX_ASSERT(request_size <= arena->size);

    return true;
}

void            
simplex_deallocate_memory_arena(MemoryArena *arena)
{

    SIMPLEX_CHECK_PTR(arena);
    if (arena->buffer != NULL)
    {
        simplex_virtual_free(arena->buffer);
    }

}

void*           
simplex_memory_arena_push_bottom(MemoryArena *arena, size_t size)
{

    SIMPLEX_CHECK_PTR(arena);
    constexpr size_t alignment = 32;
    const size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
    const size_t commit_size = simplex_memory_arena_get_commit(arena);

    SIMPLEX_ASSERT(commit_size >= aligned_size);

    void *location = static_cast<uint8_t*>(arena->buffer) + arena->offset_bottom;
    arena->offset_bottom += aligned_size;
    return location;

}

void*
simplex_memory_arena_push_top(MemoryArena *arena, size_t size)
{

    SIMPLEX_CHECK_PTR(arena);
    constexpr size_t alignment = 32;
    const size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
    const size_t commit_size = simplex_memory_arena_get_commit(arena);

    SIMPLEX_ASSERT(commit_size >= aligned_size);

    arena->offset_bottom += aligned_size;
    void *location = static_cast<uint8_t*>(arena->buffer) + (arena->size - arena->offset_bottom);

    return location;

}

void            
simplex_memory_arena_pop_bottom(MemoryArena *arena, size_t size)
{

    SIMPLEX_CHECK_PTR(arena);
    constexpr size_t alignment = 32;
    const size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
    arena->offset_bottom -= size;

}

void            
simplex_memory_arena_pop_top(MemoryArena *arena, size_t size)
{

    SIMPLEX_CHECK_PTR(arena);
    constexpr size_t alignment = 32;
    const size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
    arena->offset_top -= size;

}

arena_stamp_t   
simplex_memory_arena_stamp_bottom(MemoryArena *arena)
{
    SIMPLEX_CHECK_PTR(arena);
    return arena->offset_bottom;
}

arena_stamp_t   
simplex_memory_arena_stamp_top(MemoryArena *arena)
{
    SIMPLEX_CHECK_PTR(arena);
    return arena->offset_top;
}

void            
simplex_memory_arena_restore_bottom(MemoryArena *arena, arena_stamp_t stamp)
{
    SIMPLEX_CHECK_PTR(arena);
    arena->offset_bottom = stamp;
}

void            
simplex_memory_arena_restore_top(MemoryArena *arena, arena_stamp_t stamp)
{
    SIMPLEX_CHECK_PTR(arena);
    arena->offset_top = stamp;
}

size_t         
simplex_memory_arena_get_commit(MemoryArena *arena)
{
    SIMPLEX_CHECK_PTR(arena);
    const size_t commit = arena->offset_bottom + arena->offset_top;
    return commit;
}



#include <utils/test_registry.hpp>
#if defined(SIMPLEX_ENABLE_TESTS) && SIMPLEX_ENABLE_TESTS == 1
#include <cstdint>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Test fixtures
// ---------------------------------------------------------------------------

struct EmptyCase
{
};

struct AllocationCase
{
    size_t request_size;
    size_t alignment;
};

struct ConstructionCase
{
    int         element_count;
    const char *label;
};

static inline bool
is_aligned(const void *pointer, size_t alignment)
{
    return ((uintptr_t)pointer % alignment) == 0;
}

// A non-trivial object that records construction / destruction so we can
// verify placement-new and the destructor path through simplex_memory_*.
struct ComplexObject
{

    std::vector<int>    payload;
    std::string         label;
    int                *external;

    ComplexObject(int count, const char *name, int *destruction_flag)
        : label(name), external(destruction_flag)
    {

        for (int i = 0; i < count; ++i)
            this->payload.push_back(i * i);

    }

    ~ComplexObject()
    {

        if (this->external != nullptr)
            *this->external = 1;

    }

};

// ---------------------------------------------------------------------------
// Alignment
// ---------------------------------------------------------------------------

static bool
mem_default_alignment(const EmptyCase &)
{

    void *pointer = simplex_memory_alloc(64);
    AllocationDescriptor descriptor = simplex_memory_descriptor(pointer);

    bool ok = (pointer != nullptr)
           && is_aligned(pointer, 32)
           && (descriptor.memory_alignment == 32);

    simplex_memory_free(pointer);
    return ok;

}

static bool
mem_alignment_honored(const AllocationCase &test_case)
{

    void *pointer = simplex_memory_alloc_aligned(test_case.request_size, test_case.alignment);
    bool aligned = (pointer != nullptr) && is_aligned(pointer, test_case.alignment);
    simplex_memory_free(pointer);
    return aligned;

}

static bool
mem_descriptor_matches(const AllocationCase &test_case)
{

    void *pointer = simplex_memory_alloc_aligned(test_case.request_size, test_case.alignment);
    AllocationDescriptor descriptor = simplex_memory_descriptor(pointer);

    bool matches = (descriptor.memory_request_size == test_case.request_size)
                && (descriptor.memory_alignment == test_case.alignment)
                && (descriptor.memory_allocation_size >= test_case.request_size)
                && (descriptor.memory_base != nullptr)
                && ((descriptor.memory_allocation_size % test_case.alignment) == 0);

    simplex_memory_free(pointer);
    return matches;

}

// ---------------------------------------------------------------------------
// Object construction
// ---------------------------------------------------------------------------

static bool
mem_new_simple_pod(const EmptyCase &)
{

    int *value = simplex_memory_new<int>(42);
    bool ok = (value != nullptr) && (*value == 42) && is_aligned(value, 32);
    simplex_memory_delete(value);
    return ok;

}

static bool
mem_new_aggregate(const EmptyCase &)
{

    struct Point { float x, y, z; };
    Point *point = simplex_memory_new<Point>(Point{ 1.0f, 2.0f, 3.0f });

    bool ok = (point != nullptr)
           && (point->x == 1.0f)
           && (point->y == 2.0f)
           && (point->z == 3.0f);

    simplex_memory_delete(point);
    return ok;

}

static bool
mem_new_complex_object(const ConstructionCase &test_case)
{

    int destroyed = 0;
    ComplexObject *object = simplex_memory_new<ComplexObject>(
        test_case.element_count, test_case.label, &destroyed);

    bool constructed = (object != nullptr)
                     && (object->label == test_case.label)
                     && ((int)object->payload.size() == test_case.element_count)
                     && (destroyed == 0);

    simplex_memory_delete(object);
    return constructed && (destroyed == 1);

}

static bool
mem_new_distinct_allocations(const ConstructionCase &test_case)
{

    int count = test_case.element_count;
    std::vector<ComplexObject*> objects;
    objects.reserve(count);

    for (int i = 0; i < count; ++i)
        objects.push_back(simplex_memory_new<ComplexObject>(i + 1, test_case.label, nullptr));

    bool ok = true;
    for (int i = 0; i < count; ++i)
    {

        if (objects[i] == nullptr || (int)objects[i]->payload.size() != i + 1)
            ok = false;

        for (int j = i + 1; j < count; ++j)
            if (objects[i] == objects[j])
                ok = false;

    }

    for (ComplexObject *object : objects)
        simplex_memory_delete(object);

    return ok;

}

// ---------------------------------------------------------------------------
// Allocation / free balancing
// ---------------------------------------------------------------------------

static bool
mem_live_returns_to_baseline(const EmptyCase &)
{

    size_t live_before = simplex_memory_get_live();

    void *first  = simplex_memory_alloc(256);
    void *second = simplex_memory_alloc_aligned(512, 128);
    bool grew = simplex_memory_get_live() > live_before;

    simplex_memory_free(first);
    simplex_memory_free(second);

    return grew && (simplex_memory_get_live() == live_before);

}

static bool
mem_alloc_release_totals_track(const ConstructionCase &test_case)
{

    size_t alloc_before   = simplex_memory_get_allocations_total();
    size_t release_before = simplex_memory_get_releases_total();

    int count = test_case.element_count;
    std::vector<void*> blocks;
    blocks.reserve(count);

    for (int i = 0; i < count; ++i)
        blocks.push_back(simplex_memory_alloc(64 * (i + 1)));

    bool alloc_grew = simplex_memory_get_allocations_total() > alloc_before;

    for (void *block : blocks)
        simplex_memory_free(block);

    size_t alloc_delta   = simplex_memory_get_allocations_total() - alloc_before;
    size_t release_delta = simplex_memory_get_releases_total() - release_before;

    return alloc_grew && (alloc_delta == release_delta);

}

static bool
mem_new_delete_balanced(const ConstructionCase &test_case)
{

    size_t live_before = simplex_memory_get_live();

    int destroyed = 0;
    ComplexObject *object = simplex_memory_new<ComplexObject>(
        test_case.element_count, test_case.label, &destroyed);
    simplex_memory_delete(object);

    return (destroyed == 1) && (simplex_memory_get_live() == live_before);

}

SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "default alignment is 32",       mem_default_alignment,        EmptyCase);
SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "align 32 / size 1",             mem_alignment_honored,        AllocationCase, 1, 32);
SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "align 64 / size 100",           mem_alignment_honored,        AllocationCase, 100, 64);
SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "align 128 / size 31",           mem_alignment_honored,        AllocationCase, 31, 128);
SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "align 256 / size 4096",         mem_alignment_honored,        AllocationCase, 4096, 256);
SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "align 512 / size 1000",         mem_alignment_honored,        AllocationCase, 1000, 512);
SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "descriptor matches request",    mem_descriptor_matches,       AllocationCase, 123, 128);
SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "new: simple pod",               mem_new_simple_pod,           EmptyCase);
SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "new: aggregate",                mem_new_aggregate,            EmptyCase);
SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "new: complex object + dtor",    mem_new_complex_object,       ConstructionCase, 5, "complex");
SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "new: distinct allocations",     mem_new_distinct_allocations, ConstructionCase, 8, "obj");
SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "balance: live baseline",        mem_live_returns_to_baseline, EmptyCase);
SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "balance: alloc/release totals", mem_alloc_release_totals_track, ConstructionCase, 16, "blocks");
SIMPLEX_REGISTER_GROUPED_TEST("System: memory_alloc", "balance: new/delete",           mem_new_delete_balanced,      ConstructionCase, 10, "balance");

#endif // SIMPLEX_ENABLE_TESTS