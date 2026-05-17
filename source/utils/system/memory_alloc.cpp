#include <atomic>

#include <utils/defs.hpp>
#include <utils/system/memory_alloc.hpp>

#if defined(_WIN32)
#   include <malloc.h>
#endif

static std::atomic<size_t> memory_allocated = 0;
static std::atomic<size_t> memory_released = 0;
static std::atomic<size_t> memory_commit = 0;
static std::atomic<size_t> memory_peak = 0;

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