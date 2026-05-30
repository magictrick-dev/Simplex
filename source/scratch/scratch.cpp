#include <iostream>
#include <limits>
#include <vector>
#include <unordered_map>
#include <queue>

#include <scratch/scratch.hpp>

#include <utils/defs.hpp>
#include <utils/typeid.hpp>
#include <utils/linear.hpp>
#include <utils/system/resource_manager.hpp>
#include <utils/system/memory_alloc.hpp>

#include <simplex/array.hpp>
#include <simplex/static_array.hpp>
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
        uint16_t generation;
        uint16_t flags;

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
        virtual bool contains(const entity_t &e) const = 0;
        virtual size_t load_factor() const = 0;

};

template <typename type_t, size_t capacity>
class component_sparse_set : public component_sparse_set_interface
{

    constexpr int32_t invalid_index = -1;

    public:
        inline component_sparse_set()
        {

            for (size_t i = 0; i < capacity; ++i) 
            {
                sparse_to_dense[i] = invalid_index;
                dense_to_sparse[i] = { .identifier = 0, .generation = 0 };
            }

        }

        virtual ~component_sparse_set() = default;

        virtual inline entity_t* begin() override               { return this->elements.begin();    }
        virtual inline entity_t* end() override                 { return this->elements.end();      }
        virtual inline const entity_t* begin() const override   { return this->elements.begin();    }
        virtual inline const entity_t* end() const override     { return this->elements.end();      }
        virtual inline size_t load_factor() const override      { return this->elements.size();     }

        template <typename... Args> inline void 
        insert(const entity_t& e, Args&&... args)
        {

            const size_t sparse_index = e.identifier % capacity;
            const size_t dense_index = this->sparse_to_dense[sparse_index];

            // NOTE(Chris): We expect that the API checks that the entity exists first,
            //              as emplacement assumes a new value.
            SIMPLEX_ASSERT(dense_index == invalid_index);
            const size_t insertion_index = this->elements.size();
            this->elements.emplace_back(std::forward<Args>(args)...);

            this->sparse_to_dense[sparse_index] = insertion_index;
            this->dense_to_sparse[insertion_index] = e;

        }

        inline void
        remove(const entity_t& e)
        {

        }

        virtual inline bool
        contains(const entity_t& e) const
        {

            const size_t sparse_index = e.identifer % capacity;
            const size_t dense_index = this->sparse_to_dense[sparse_index];
            
            if (dense_index == invalid_index) return false;
            return (this->dense_to_sparse[dense_index] == e);
            
        }

        inline type_t& operator[](size_t dense_index) { return this->elements[dense_index]; }
        inline const type_t& operator[](size_t dense_index) const { return this->elements[dense_index]; }

    private:
        spx::array<entity_t, capacity>          dense_to_sparse;
        spx::array<int32_t, capacity>           sparse_to_dense;
        spx::static_array<type_t, capacity>     elements;
        
};

template <size_t pages, size_t capacity>
struct paged_components
{

    public:
        inline component_sparse_set_interface* operator[](size_t index) { return this->interfaces[index]; }
        inline const component_sparse_set_interface* operator[](size_t index) const { return this->interfaces[index]; }
    
        inline bool 
        page_is_allocated(size_t page_index) const
        {

            const bool result = (interfaces[page_index] != NULL);
            return result;

        }
    
        template <typename type_t> inline void 
        allocate_page(const size_t index)
        {  

            if (interfaces[index] == nullptr)
            {
                interfaces[index] = simplex_memory_new<component_sparse_set<type_t, capacity>>();
            }

        }

        template <typename type_t> inline void
        deallocate_page(const size_t index)
        {
            if (interfaces[index] != nullptr)
            {
                component_sparse_set<type_t, capacity> *component_set =
                    reinterpret_cast<component_sparse_set<type_t, capacity>>(interfaces[index]);
                simplex_memory_delete(component_set);
                interfaces[index] = nullptr;
            }
        }
    
    private:
        spx::array<component_sparse_set_interface*, pages> interfaces;


};

class EntitySystem
{

    public:
        static inline EntitySystem& Get()
        {
            static EntitySystem system = {};
            return system;
        }

        template <typename type_t> inline void 
        register_component()
        {

            constexpr size_t type_hash = TypeID<type_t>::Hash;
            if (this->component_pages.find(type_hash) != this->component_pages.end()) return;

            this->component_pages[type_hash] = {};
            //this->attach_component<type_t>(this->entity_placeholder);
            //this->attach_component<type_t>(this->entity_trap);

        }

        inline entity_t 
        create_entity()
        {

            entity_t result_entity = { };

            if (!entities_inactive.empty())
            {
                result_entity = entities_inactive.front();
                entities_inactive.pop();
            }
            else
            {
                const size_t entity_identifier = entities_active.size();
                entities_active.emplace_back();
                result_entity = entities_active.back();
                result_entity.identifier = entity_identifier;
                result_entity.generation = 0;
            }

            result_entity.flags = 0;
            return result_entity;

        }

        inline void
        destroy_entity(const entity_t &e)
        {

            const size_t entity_index = e.identifier;
            if (this->entities_active[entity_index] != e) return;
            this->entities_active[entity_index].generation++;
            this->entities_inactive.push(e);

        }

        template <typename type_t, typename... Args> type_t& 
        attach_component(const entity_t &e, Args&&... args)
        {
            return {};
        }

    private:
        EntitySystem()
        {
            entity_placeholder = this->create_entity();
            entity_trap = this->create_entity();
        }

        ~EntitySystem()
        {

            return;
        }

        inline EntitySystem(EntitySystem &copy) = delete;
        inline EntitySystem& operator==(EntitySystem &&other) = delete;

    private:
        entity_t entity_placeholder;
        entity_t entity_trap;
        std::vector<entity_t> entities_active;
        std::queue<entity_t> entities_inactive;
        std::unordered_map<size_t, paged_components<64, 4096>> component_pages;

};

int 
scratch_main()
{

    auto& entity_system = EntitySystem::Get();
    entity_system.register_component<spx::metadata>();
    entity_system.register_component<spx::transform>();
    entity_system.register_component<spx::mesh>();
    entity_system.register_component<spx::material>();

    return 0;
}