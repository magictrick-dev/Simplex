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
#include <scratch/sparseset.hpp>

struct Metadata
{
    std::string name;
    std::string uuid;
    entity_t self;
};

struct Transform
{
    vec3f position;
    vec3f rotation;
    vec3f scale;
};

struct Mesh
{
    ResourceHandle handle;
};

struct Material
{
    ResourceHandle texture;
    ResourceHandle normal;
    ResourceHandle bump;
    real32_t Ka;
    real32_t Kd;
    real32_t Ks;
    real32_t Ke;
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
        std::unordered_map<size_t, SparseSetInterface*> components;

};

int 
scratch_main()
{

    auto& entity_system = EntitySystem::Get();
    entity_system.register_component<Metadata>();
    entity_system.register_component<Transform>();
    entity_system.register_component<Mesh>();
    entity_system.register_component<Material>();

    return 0;
}