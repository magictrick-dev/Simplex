#include <iostream>
#include <limits>
#include <vector>
#include <unordered_map>

#include <scratch/nameof.hpp>
#include <scratch/scratch.hpp>
#include <utils/defs.hpp>
#include <utils/typeid.hpp>

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

template <typename T>
class SparseSet
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



template <typename Dummy>
struct Foo
{
    Dummy aspect;
    int32_t width;
    int32_t height;
};

class Shape
{
    public:
        Shape() = default;
        virtual ~Shape() = default;

        virtual int32_t area() { return _width * _height; }

    protected:
        int32_t _width;
        int32_t _height;
};

class Square : public Shape
{
    public:
        Square(int32_t side) { this->_width = side; this->_height = side; };
        virtual ~Square() = default;

        virtual int32_t area() override { return _width * _width; }
};

union Testunion
{
    int64_t packed;
    struct
    {
        int32_t left;
        int32_t right;
    };
};

typedef std::vector<float> vectorf;

int 
scratch_main()
{

    float foo_bar = 32;
    std::cout << TypeID<float>::Value << std::endl;
    std::cout << TypeID<Shape>::Value << std::endl;
    std::cout << TypeID<Foo<float>>::Value << std::endl;
    std::cout << TypeID<Testunion>::Value << std::endl;
    std::cout << TypeID<vectorf>::Value << std::endl;

    auto list = TypeIDArray<float, int32_t, Shape>::Values;
    auto hashes = TypeIDArray<float, int32_t, Shape>::Hashes;
    for (auto l : list) std::cout << l << ", "; std::cout << std::endl;
    for (auto h : hashes) std::cout << h << ", "; std::cout << std::endl;

    return 0;
}