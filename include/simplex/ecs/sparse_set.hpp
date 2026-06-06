#pragma once
#include <simplex/ecs/entity.hpp>
#include <simplex/static_array.hpp>
#include <simplex/array.hpp>

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

template <typename type_t, size_t capacity> inline void
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