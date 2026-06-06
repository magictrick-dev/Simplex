#pragma once
#include <simplex/ecs/entity.hpp>
#include <simplex/ecs/sparse_set.hpp>
#include <simplex/ecs/component_view.hpp>
#include <simplex/static_array.hpp>
#include <simplex/static_queue.hpp>
#include <simplex/hashed_sparse_map.hpp>

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
            (SIMPLEX_CHECK_PTR(get_component_sparse_set<Components>()), ...);
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