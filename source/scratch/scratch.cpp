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
        virtual    ~component_sparse_set_interface() = default;

};

template <typename type_t, size_t capacity>
class component_sparse_set : public component_sparse_set_interface
{

    public:
                    component_sparse_set() = default;
        virtual    ~component_sparse_set() = default;

    private:
        spx::array<entity_t, capacity>          dense_to_sparse;
        spx::array<int32_t, capacity>           sparse_to_dense;
        spx::static_array<type_t, capacity>     elements;
        
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

    private:
        EntitySystem()
        {

        }

        ~EntitySystem()
        {

        }

        inline EntitySystem(EntitySystem &copy) = delete;
        inline EntitySystem& operator==(EntitySystem &&other) = delete;

    private:
        entity_t entity_placeholder;        // For silent-but-okay fails.
        entity_t entity_trap;               // For forced trigger fails.

        std::vector<entity_t> entities_active;
        std::queue<entity_t> entities_inactive;
        std::unordered_map<size_t, component_sparse_set_interface*> components;

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