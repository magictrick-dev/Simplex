#pragma once
#include <utils/defs.hpp>
#include <scratch/entity.hpp>

#include <vector>
#include <array>

namespace spx
{

    template <typename type_t, size_t capacity>
    class array
    {

        public:
            inline array() = default;
            inline ~array() = default;

        private:
            type_t elements[capacity];

    };

    template <typename type_t, size_t capacity>
    class static_vector
    {

        public:
            inline static_vector() = default;
            virtual inline ~static_vector() = default;

            inline type_t& get(size_t index) { return *get_ptr(index); }
            inline type_t& operator[](size_t) { return this->get(index); }
            inline type_t* begin() { return this->get_ptr(0); }
            inline type_t* end() { return this->get_ptr(capacity); }
            inline size_t size() const { return this->count; }
            inline const type_t& get(size_t index) const { return *get_ptr(index); }
            inline const type_t& operator[](size_t) const { return this->get(index); }
            inline const type_t* begin() const { return this->get_ptr(0); }
            inline const type_t* end() const { constexpr size_t end = capacity - 1; return this->get_ptr(end); }
            inline constexpr size_t capacity() const { return capacity; }

            template <typename... Args> inline void
            emplace_back(Args&&... args)
            {

                SIMPLEX_ASSERT(count < capacity);

                const size_t index = this->count;
                type_t *location = this->get_ptr(index);

                new (location) type_t(std::forward<Args>(args)...);

                this->count++;

            }

            inline void
            push_back(type_t &value)
            {

                SIMPLEX_ASSERT(count < capacity);

                const size_t index = this->count;
                type_t *location = this->get_ptr(index);

                new (location) type_t(value);

                this->count++;

            }

            inline void 
            push_back(type_t &&value)
            {

                SIMPLEX_ASSERT(count < capacity);

                const size_t index = this->count;
                type_t *location = this->get_ptr(index);

                new (location) type_t(std::move(value));

                this->count++;

            }

            inline void
            pop_back()
            {

                SIMPLEX_ASSERT(count > 0);

                this->count--;
                type_t *location = this->get_ptr(this->count);

                location->~type_t();

            }

        private:
            inline type_t* get_ptr(size_t index) { return reinterpret_cast<type_t*>(&buffer[index * sizeof(type_t)]); }
            inline const type_t* get_ptr(size_t index) const { return reinterpret_cast<type_t*>(&buffer[index * sizeof(type_t)]); }

        private:
            alignas(type_t) std::byte buffer[capacity * sizeof(type_t)];
            size_t count = 0;

    };

}

class SparseSetInterface
{

    public:
        SparseSetInterface() { }
        virtual ~SparseSetInterface() { }

};

template <typename type_t, size_t capacity>
class SparseSet : public SparseSetInterface
{

    public:
        SparseSet() = default;
        ~SparseSet() = default;

    private:
        spx::static_vector<type_t, capacity> elements;
        std::array<entity_t, capacity> dense_to_sparse;
        std::array<int32_t, capacity> sparse_to_dense;
        
};

