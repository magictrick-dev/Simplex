#include <iostream>
#include <limits>
#include <vector>
#include <unordered_map>

#include <scratch/scratch.hpp>
#include <utils/defs.hpp>

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

int 
scratch_main()
{
    
    SparseSet<std::string> my_sparse_set;

    my_sparse_set.insert(17,    "Hello");
    my_sparse_set.insert(13,    "World");
    my_sparse_set.insert(2,     "Foo");
    my_sparse_set.insert(26,    "Bar");
    my_sparse_set.insert(25,    "Baz");

    std::cout << std::endl << "-------------------" << std::endl;
    for (size_t i = 0; i < my_sparse_set.size(); ++i)
    {
        const size_t sparse_index = my_sparse_set.get_sparse_from_dense(i);
        const size_t dense_index = my_sparse_set.get_dense_from_sparse(sparse_index);
        std::cout << my_sparse_set[i] << std::endl;
        std::cout << "       Iter. Index: " << i << std::endl;
        std::cout << "      Sparse Index: " << sparse_index << std::endl;
        std::cout << "       Dense Index: " << dense_index << std::endl;
    }

    my_sparse_set.remove(2);

    std::cout << std::endl << "-------------------" << std::endl;
    for (size_t i = 0; i < my_sparse_set.size(); ++i)
    {
        const size_t sparse_index = my_sparse_set.get_sparse_from_dense(i);
        const size_t dense_index = my_sparse_set.get_dense_from_sparse(sparse_index);
        std::cout << my_sparse_set[i] << std::endl;
        std::cout << "       Iter. Index: " << i << std::endl;
        std::cout << "      Sparse Index: " << sparse_index << std::endl;
        std::cout << "       Dense Index: " << dense_index << std::endl;
    }

    my_sparse_set.remove(17);

    std::cout << std::endl << "-------------------" << std::endl;
    for (size_t i = 0; i < my_sparse_set.size(); ++i)
    {
        const size_t sparse_index = my_sparse_set.get_sparse_from_dense(i);
        const size_t dense_index = my_sparse_set.get_dense_from_sparse(sparse_index);
        std::cout << my_sparse_set[i] << std::endl;
        std::cout << "       Iter. Index: " << i << std::endl;
        std::cout << "      Sparse Index: " << sparse_index << std::endl;
        std::cout << "       Dense Index: " << dense_index << std::endl;
    }

    my_sparse_set.remove(25);

    std::cout << std::endl << "-------------------" << std::endl;
    for (size_t i = 0; i < my_sparse_set.size(); ++i)
    {
        const size_t sparse_index = my_sparse_set.get_sparse_from_dense(i);
        const size_t dense_index = my_sparse_set.get_dense_from_sparse(sparse_index);
        std::cout << my_sparse_set[i] << std::endl;
        std::cout << "       Iter. Index: " << i << std::endl;
        std::cout << "      Sparse Index: " << sparse_index << std::endl;
        std::cout << "       Dense Index: " << dense_index << std::endl;
    }

    return 0;
}