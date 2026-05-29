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

/// SDSA Allocator Interface
/** 
 * Provides a way of interacting with custom allocators when using Simplex datastructures.
 */
struct SDSA_AllocatorInterface
{

    /// @brief Allocates at least n-bytes from a derived implementation.
    /// @param size The request size to allocate
    /// @return If the return value is NULL, the allocation failed.
    ///
    /// The allocate method should defer to allocate_aligned with an alignment of 16.
    inline virtual void*    allocate(const size_t size) = 0;
    inline virtual void*    allocate_aligned(const size_t size, const size_t alignment) = 0;

    inline virtual void*    reallocate(void *buffer, const size_t size) = 0;
    inline virtual void*    reallocate_aligned(void *buffer, const size_t size, const size_t alignment) = 0;

    inline virtual void     deallocate() = 0;

};

struct SDSA_SimplexDefaultAllocator
{

};

struct SDSA_SimplexMemoryArenaAllocator
{

};

class SparseSetInterface
{

    public:
        SparseSetInterface() { }
        virtual ~SparseSetInterface() { }


};

template <typename T>
class SparseSet : public SparseSetInterface
{

    public:
        SparseSet() = default;
        ~SparseSet() = default;

        inline std::vector<T>::iterator begin()                 { return this->elements.begin();    }
        inline std::vector<T>::iterator end()                   { return this->elements.end();      }
        inline std::vector<T>::const_iterator begin() const     { return this->elements.begin();    }
        inline std::vector<T>::const_iterator end() const       { return this->elements.end();      }
        inline T& operator[](const size_t index)                { return this->elements[index];     }
        inline const T& operator[](const size_t index) const    { return this->elements[index];     }
        inline size_t size() const                              { return this->elements.size();     }

        inline void
        insert(size_t sparse_index, T value)
        {

            SIMPLEX_ASSERT(!this->exists(sparse_index));

            const size_t dense_index = this->elements.size();

            this->elements.emplace_back(value);
            this->sparse_to_dense[sparse_index]     = dense_index;
            this->dense_to_sparse[dense_index]      = sparse_index;

        }

        inline T& 
        get(size_t sparse_index)
        {
            return this->elements[sparse_to_dense[sparse_index]];
        }

        inline const T& 
        get(size_t sparse_index) const
        {
            return this->elements[sparse_to_dense[sparse_index]];
        }

        inline bool
        exists(size_t sparse_index) const
        {
            auto it = sparse_to_dense.find(sparse_index);
            const bool result = (it != sparse_to_dense.end());
            return result;
        }

        inline void
        remove(size_t sparse_index)
        {

            SIMPLEX_ASSERT(this->exists(sparse_index));
            const size_t remove_dense_index     = this->sparse_to_dense[sparse_index];
            const size_t swap_dense_index       = this->elements.size() - 1;
            const size_t swap_sparse_index      = this->dense_to_sparse[swap_dense_index];
            std::cout << "Swap sparse is: " << swap_sparse_index << std::endl;

            // If the thing we remove is the back, removal is trivial.
            if (swap_dense_index == remove_dense_index)
            {
                this->sparse_to_dense.erase(sparse_index);
                this->dense_to_sparse.erase(remove_dense_index);
                this->elements.pop_back();
                return;
            }
            
            // Move the element from the end and move it into the thing we are removing.
            this->elements[remove_dense_index] = this->elements[swap_dense_index];

            this->dense_to_sparse[remove_dense_index] = swap_sparse_index;
            this->sparse_to_dense[swap_sparse_index] = remove_dense_index;
            
            // Finally, remove the element.
            this->elements.pop_back();
            
        }

        inline size_t 
        get_dense_from_sparse(const size_t index) const
        {
            const auto it = this->sparse_to_dense.find(index);
            if (it != this->sparse_to_dense.end())
            {
                return it->second;
            }
            SIMPLEX_NO_REACH("Attempting to access a non-existent sparse index.");
            return std::numeric_limits<size_t>::max();
        }

        inline size_t 
        get_sparse_from_dense(const size_t index) const
        {
            const auto it = this->dense_to_sparse.find(index);
            if (it != this->dense_to_sparse.end())
            {
                return it->second;
            }
            SIMPLEX_NO_REACH("Attempting to access a non-existent dense index.");
            return std::numeric_limits<size_t>::max();
        }


    private:
        std::vector<T> elements;
        std::unordered_map<size_t,size_t> sparse_to_dense;
        std::unordered_map<size_t,size_t> dense_to_sparse;
        

};

union EntityID
{

    uint64_t handle;

    struct
    {

        uint32_t identifier;
        uint16_t generation;
        uint16_t flags;

    };

};


struct Metadata
{
    std::string name;
    std::string uuid;
    EntityID self;
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

            auto &self = Get();

            constexpr size_t type_hash = TypeID<T>::Hash;
            auto it = self.components.find(type_hash);

            // NOTE(Chris): Not really a fan of a conditional here, but it prevents stupid
            //              double-allocation behaviors.
            if (it == self.components.end())
            {

                SparseSet<T>* component_sparse_set = simplex_memory_new<SparseSet<T>>();
                component_sparse_set.insert(self.entity_placeholder.identifier);
                component_sparse_set.insert(self.entity_trap.identifier);
                self.components[type_hash] = component_sparse_set;

            }
            else
            {
                // TODO(Chris): Should we just silently fail?
                SIMPLEX_ASSERT("EntitySystem attempted to double register a component!");
            }


        }

    private:
        EntitySystem()
        {

            this->entity_placeholder    = { .identifier = 0, .generation = 0 };
            this->entity_trap           = { .identifier = 1, .generation = 0 };
            this->entities.push_back(entity_placeholder);
            this->entities.push_back(entity_trap);

        }

        ~EntitySystem()
        {

        }

        inline EntitySystem(EntitySystem &copy) = delete; // Singleton enforcement, no copy.
        inline EntitySystem& operator==(EntitySystem &&other) = delete; // Singleton enforcement, no hacky copy.

    private:
        EntityID entity_placeholder;        // For silent-but-okay fails.
        EntityID entity_trap;               // For forced trigger fails.

        std::vector<EntityID> entities;
        std::queue<EntityID> compost;
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