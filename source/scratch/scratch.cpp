#include <iostream>
#include <limits>
#include <vector>
#include <unordered_map>
#include <queue>
#include <string>
#include <tuple>

#include <scratch/scratch.hpp>

#include <utils/defs.hpp>
#include <utils/typeid.hpp>
#include <utils/linear.hpp>
#include <utils/system/resource_manager.hpp>
#include <utils/system/memory_alloc.hpp>

#include <simplex/array.hpp>
#include <simplex/static_array.hpp>
#include <simplex/string_view.hpp>
#include <simplex/array_view.hpp>
#include <simplex/dynamic_string.hpp>
#include <simplex/static_string.hpp>
#include <simplex/static_queue.hpp>
#include <simplex/hashed_sparse_map.hpp>

#include <simplex/components/metadata.hpp>
#include <simplex/components/transform.hpp>
#include <simplex/components/mesh.hpp>
#include <simplex/components/material.hpp>

union entity_t
{


    uint64_t handle;

    struct
    {

        uint32_t identifier;
        uint32_t generation;

    };


    inline operator uint64_t()                          { return handle;                        }
    inline entity_t& operator=(uint64_t handle)         { this->handle = handle; return *this;  }
    inline bool operator==(const entity_t&other) const  { return this->handle == other.handle;  }
    inline bool operator!=(const entity_t &other) const { return !(*this == other);             }

};

class component_sparse_set_interface
{

    public:
                 component_sparse_set_interface() = default;
        virtual ~component_sparse_set_interface() = default;

        virtual entity_t* begin() = 0;
        virtual entity_t* end() = 0;
        virtual const entity_t* begin() const = 0;
        virtual const entity_t* end() const = 0;
        virtual size_t count() const = 0;

};

template <typename type_t, size_t capacity>
class component_sparse_set : public component_sparse_set_interface
{


    public:
        static inline constexpr int32_t invalid_index = -1;

        inline component_sparse_set()
        {

            for (size_t i = 0; i < capacity; ++i) 
            {
                sparse_to_dense[i] = invalid_index;
            }

        }

        virtual ~component_sparse_set() = default;

        virtual inline entity_t* begin() override               { return this->dense_to_sparse.begin(); }
        virtual inline entity_t* end() override                 { return this->dense_to_sparse.end();   }
        virtual inline const entity_t* begin() const override   { return this->dense_to_sparse.begin(); }
        virtual inline const entity_t* end() const override     { return this->dense_to_sparse.end();   }
        virtual inline size_t count() const override            { return this->elements.size();         }

        template <typename... Args> inline void 
        insert(const entity_t& e, Args&&... args)
        {

            const size_t sparse_index = e.identifier;
            SIMPLEX_ASSERT(sparse_index < capacity);

            const size_t dense_index = this->sparse_to_dense[sparse_index];

            // NOTE(Chris): We expect that the API checks that the entity exists first,
            //              as emplacement assumes a new value.
            SIMPLEX_ASSERT(dense_index == invalid_index);
            const size_t insertion_index = this->elements.size();
            this->elements.emplace_back(std::forward<Args>(args)...);

            this->sparse_to_dense[sparse_index] = insertion_index;
            this->dense_to_sparse.emplace_back(e);

        }

        inline void
        remove(const entity_t& e)
        {

            const size_t sparse_index = e.identifier;
            SIMPLEX_ASSERT(sparse_index < capacity);

            const size_t dense_index = this->sparse_to_dense[sparse_index];
            
            // NOTE(Chris): We expect that the API checks that the entity exists first,
            //              as removal assumes a valid and expected value.
            SIMPLEX_ASSERT(dense_index != invalid_index);
            const entity_t &expected = this->dense_to_sparse[dense_index];
            SIMPLEX_ASSERT(expected == e);

            const size_t last_dense_index = this->elements.size() - 1;
            const entity_t last_entity = this->dense_to_sparse[last_dense_index];
            const size_t last_sparse_index = last_entity.identifier;

            if (last_dense_index == dense_index)
            {

                this->elements.pop_back();
                this->dense_to_sparse.pop_back();
                this->sparse_to_dense[sparse_index] = invalid_index;

            }

            else
            {

                this->elements[dense_index] = std::move(this->elements[last_dense_index]);
                this->dense_to_sparse[dense_index] = last_entity;
                this->sparse_to_dense[last_sparse_index] = dense_index;
                this->elements.pop_back();
                this->dense_to_sparse.pop_back();

            }
            

        }

        inline bool
        contains(const entity_t& e) const
        {

            const size_t sparse_index = e.identifier;
            SIMPLEX_ASSERT(sparse_index < capacity);

            const size_t dense_index = this->sparse_to_dense[sparse_index];
            
            if (dense_index == invalid_index) return false;
            return (this->dense_to_sparse[dense_index] == e);
            
        }

        inline type_t& 
        get(const entity_t &e) 
        {

            const size_t sparse_index = e.identifier;
            SIMPLEX_ASSERT(sparse_index < capacity);

            const int32_t dense_index = this->sparse_to_dense[sparse_index];
            SIMPLEX_ASSERT(dense_index != invalid_index);
            SIMPLEX_ASSERT(this->dense_to_sparse[dense_index] == e);
            return this->elements[dense_index];

        }

        inline const type_t& 
        get(const entity_t &e) const
        {

            const size_t sparse_index = e.identifier;
            SIMPLEX_ASSERT(sparse_index < capacity);

            const int32_t dense_index = this->sparse_to_dense[sparse_index];
            SIMPLEX_ASSERT(dense_index != invalid_index);
            SIMPLEX_ASSERT(this->dense_to_sparse[dense_index] == e);
            return this->elements[dense_index];

        }

        inline int32_t
        to_dense_index(const entity_t &e) const
        {

            const size_t sparse_index = e.identifier;
            SIMPLEX_ASSERT(sparse_index < capacity);

            const size_t dense_index = this->sparse_to_dense[sparse_index];
            return dense_index;

        }

        inline entity_t
        to_sparse_index(const int32_t dense_index) const
        {

            if (dense_index >= this->dense_to_sparse.size()) 
                return { .identifier = 0, .generation = 0 };
            return this->dense_to_sparse[dense_index];

        }

        inline type_t& operator[](size_t dense_index) { return this->elements[dense_index]; }
        inline const type_t& operator[](size_t dense_index) const { return this->elements[dense_index]; }

    private:
        spx::array<int32_t, capacity>           sparse_to_dense;
        spx::static_array<entity_t, capacity>   dense_to_sparse;
        spx::static_array<type_t, capacity>     elements;

};

/// @brief A lazy, filtered range over entities that have ALL of Components...
///        Drives iteration from the smallest pool and skips entities missing any component.
/// @tparam capacity   Must match the capacity the component_sparse_sets were created with.
/// @tparam Components The required component types.
template <size_t capacity, typename... Components>
class component_view
{

    public:
        // Built by the entity_system, which resolves each pool pointer (asserting non-null).
        inline component_view(component_sparse_set<Components, capacity>*... pool_ptrs)
            : pools(pool_ptrs...)
        {

            // Pick the lead (smallest) pool once. dense_to_sparse is entity_t for every
            // pool, so we can iterate any pool's entity span uniformly regardless of which
            // component type ends up being the lead.
            size_t best = static_cast<size_t>(-1);
            ([&]
            {
                const size_t c = pool_ptrs->count();
                if (c < best)
                {
                    best       = c;
                    lead_begin = pool_ptrs->begin();
                    lead_end   = pool_ptrs->end();
                }
            }(), ...);

        }

        class iterator
        {

            public:
                inline iterator(const component_view* parent, const entity_t* cursor)
                    : view(parent), current(cursor)
                {
                    this->advance_to_match();
                }

                inline iterator& operator++()
                {
                    ++this->current;
                    this->advance_to_match();
                    return *this;
                }

                inline bool operator!=(const iterator& other) const { return this->current != other.current; }
                inline bool operator==(const iterator& other) const { return this->current == other.current; }

                // Yields (entity, Components&...). References stay valid until the next
                // structural change to any participating pool.
                inline std::tuple<entity_t, Components&...>
                operator*() const { return this->view->dereference(*this->current); }

            private:
                inline void
                advance_to_match()
                {
                    while (this->current != this->view->lead_end && !this->view->matches(*this->current))
                        ++this->current;
                }

            private:
                const component_view*   view;
                const entity_t*         current;

        };

        inline iterator begin() const { return iterator(this, lead_begin); }
        inline iterator end()   const { return iterator(this, lead_end);   }

    private:
        // True iff e is present in every pool. The lead pool always passes (one redundant,
        // cheap lookup per matched entity); kept for uniformity since the lead is a runtime choice.
        inline bool
        matches(const entity_t& e) const
        {
            return std::apply([&](auto*... ps) { return (ps->contains(e) && ...); }, pools);
        }

        inline std::tuple<entity_t, Components&...>
        dereference(const entity_t& e) const
        {
            return std::apply(
                [&](auto*... ps) -> std::tuple<entity_t, Components&...>
                { return std::tuple<entity_t, Components&...>(e, ps->get(e)...); },
                pools);
        }

    private:
        std::tuple<component_sparse_set<Components, capacity>*...>   pools;
        const entity_t*                                             lead_begin = nullptr;
        const entity_t*                                             lead_end   = nullptr;

};

template <typename type_t, size_t capacity> static inline void
dump_sparse_set(component_sparse_set<type_t, capacity> &set)
{

    for (const entity_t &e : set)
    {

        const size_t supposed_sparse = e.identifier;
        const size_t actual_dense = set.to_dense_index(e);
        const size_t actual_sparse = set.to_sparse_index(actual_dense);
        const spx::string_view<char> message = set.get(e);

        std::cout << message.data() << "(" << supposed_sparse << ") Sparse: " << actual_sparse << " Dense: " << actual_dense << "\n";

    }

    std::cout << std::endl;

}

/// @brief A captured group of entities mapped by type-hashes.
/// @tparam capacity The maximum number of entities allowed.
class entity_system
{

    constexpr static inline size_t capacity = 4096;

    public:
        static inline entity_system& get_system()
        {
            static entity_system system = {};
            return system;
        }

        inline entity_t 
        create_entity()
        {

            SIMPLEX_ASSERT(entities_active.size() < capacity); // Oops, too many entities.
            // TODO

            return {};

        }

        inline void 
        destroy_entity(const entity_t &entity)
        {

            // TODO

        }

        template <typename component_type_t> inline void
        register_component()
        {

            if (get_component_sparse_set<component_type_t>() != nullptr) return;

            constexpr uint64_t component_hash = TypeID<component_type_t>::Hash;
            component_sparse_set<component_type_t, capacity> *component_set = 
                simplex_memory_new<component_sparse_set<component_type_t, capacity>>();

            components[component_hash] = component_set;

        }

        template <typename component_type_t, typename... Args> inline component_type_t&
        attach_component_to(entity_t entity, Args&&... args)
        {

            // TODO

            
        }

        template <typename... Components> inline component_view<capacity, Components...>
        view()
        {

            // Every pool must be registered first; assert resolves to non-null.
            ((SIMPLEX_ASSERT(get_component_sparse_set<Components>() != nullptr)), ...);
            return component_view<capacity, Components...>(get_component_sparse_set<Components>()...);

        }

    private:
        inline  entity_system() = default;
        inline  entity_system(const entity_system& other) = delete;
        inline  entity_system(entity_system&& other) = delete;
        inline ~entity_system()
        {
            // TODO(Chris): Delete the component sets... or do we? Application is exitting anyway.
        }

        inline entity_system& operator=(const entity_system& other) = delete;
        inline entity_system& operator=(entity_system&& other) = delete;

    private:
        inline void
        initialize_entity(const entity_t &entity)
        {

            // Enforce certain component attachments, like metadata.

        }

        template <typename component_type_t> inline component_sparse_set<component_type_t, capacity>*
        get_component_sparse_set()
        {

            constexpr uint64_t component_hash = TypeID<component_type_t>::Hash;
            if (components.contains(component_hash))
            {
                return dynamic_cast<component_sparse_set<component_type_t, capacity>*>(components[component_hash]);
            }

            return nullptr;

        }

    private:
        spx::static_array<entity_t, capacity> entities_active;
        spx::static_queue<entity_t, capacity> entities_inactive;
        spx::hashed_sparse_map<uint64_t, component_sparse_set_interface*> components;

};

int 
scratch_main()
{

    entity_system &system = entity_system::get_system();
    system.register_component<spx::metadata>();
    system.register_component<spx::transform>();
    system.register_component<spx::material>();
    system.register_component<spx::mesh>();

    return 0;
}