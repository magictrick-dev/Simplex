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

#include <scratch/entity.hpp>
#include <simplex/array.hpp>
#include <simplex/static_array.hpp>
#include <simplex/components/metadata.hpp>
#include <simplex/components/transform.hpp>
#include <simplex/components/mesh.hpp>
#include <simplex/components/material.hpp>

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


};

template <typename type_t, size_t capacity>
class component_sparse_set : public component_sparse_set_interface
{

    public:
                 component_sparse_set() = default;
        virtual ~component_sparse_set() = default;

        virtual inline entity_t* begin() override          { return this->elements.begin();     }
        virtual inline entity_t* end() override            { return this->elements.end();       }
        virtual inline const entity_t* begin() const       { return this->elements.begin();     }
        virtual inline const entity_t* end() const         { return this->elements.end();       }

        virtual inline bool
        contains(const entity_t &e) const
        {

            const size_t lookup = e.identifier % capacity;
            const int32_t result = sparse_to_dense[lookup];
            if (result == -1) return false;

            return dense_to_sparse[result] == e;
            
        }

        inline type_t&
        get_from_sparse(const entity_t &e)
        {

            const size_t lookup = e.identifier % capacity;
            const int32_t location = sparse_to_dense[lookup];
            SIMPLEX_ASSERT(lookup != -1);
            return elements[location];

        }

        inline const type_t&
        get_from_sparse(const entity_t &e) const
        {

            const size_t lookup = e.identifier % capacity;
            const int32_t location = sparse_to_dense[lookup];
            SIMPLEX_ASSERT(lookup != -1);
            return elements[location];

        }

        inline type_t& operator[](size_t dense_index) { return this->elements[dense_index]; }
        inline const type_t& operator[](size_t dense_index) const { return this->elements[dense_index]; }

    private:
        spx::array<entity_t, capacity>          dense_to_sparse;
        spx::array<int32_t, capacity>           sparse_to_dense;
        spx::static_array<type_t, capacity>     elements;
        
};

template <size_t pages, size_t capacity>
struct paged_entity_component
{

    public:
        inline component_sparse_set_interface* operator[](size_t index) { return this->interfaces[index]; }
        inline const component_sparse_set_interface* operator[](size_t index) const { return this->interfaces[index]; }
    
        inline bool 
        entity_to_page_exists(const entity_t &e) const
        {

            const size_t page = e.identifier / capacity;
            const bool result = (interfaces[page] != NULL);
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

        template <typename T> inline void 
        register_component()
        {

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

        }

    private:
        EntitySystem()
        {

            return;
        }

        ~EntitySystem()
        {

            return;
        }

        inline void initialize_entity(const entity_t &e)
        {

            return;
        }

        inline EntitySystem(EntitySystem &copy) = delete;
        inline EntitySystem& operator==(EntitySystem &&other) = delete;

    private:
        std::vector<entity_t> entities_active;
        std::queue<entity_t> entities_inactive;
        std::unordered_map<size_t, std::vector<component_sparse_set_interface*>> component_pages;

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