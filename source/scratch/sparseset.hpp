#pragma once
#include <utils/defs.hpp>
#include <scratch/entity.hpp>

#include <vector>
#include <array>

template <typename type_t, size_t capacity>
class StaticVector
{

    public:
        inline StaticVector() = default;
        virtual inline ~StaticVector() = default;

        inline type_t& get(size_t index) { return *get_ptr(index); }
        inline type_t& operator[](size_t) { return this->get(index); }
        inline const type_t& get(size_t index) const { return *get_ptr(index); }
        inline const type_t& operator[](size_t) const { return this->get(index); }

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

class SparseSetInterface
{

    public:
        SparseSetInterface() { }
        virtual ~SparseSetInterface() { }

        virtual size_t size() const = 0;
        virtual std::vector<entity_t>::iterator begin() = 0;
        virtual std::vector<entity_t>::iterator end() = 0;
        virtual std::vector<entity_t>::const_iterator begin() const = 0;
        virtual std::vector<entity_t>::const_iterator end() const = 0;

};

template <typename type_t, size_t capacity>
class SparseSet : public SparseSetInterface
{

    public:
        SparseSet() = default;
        ~SparseSet() = default;

        inline type_t& operator[](const size_t index)                               { return this->elements[index];         }
        inline const type_t& operator[](const size_t index) const                   { return this->elements[index];         }
        virtual inline size_t size() const override                                 { return this->elements.size();         }
        virtual inline std::vector<entity_t>::iterator begin() override             { return this->dense_to_sparse.begin(); }
        virtual inline std::vector<entity_t>::iterator end() override               { return this->dense_to_sparse.end();   }
        virtual inline std::vector<entity_t>::const_iterator begin() const override { return this->dense_to_sparse.begin(); }
        virtual inline std::vector<entity_t>::const_iterator end() const override   { return this->dense_to_sparse.end();   }

    private:
        StaticVector<type_t, capacity> elements;
        StaticVector<entity_t, capacity> dense_to_sparse;
        StaticVector<int32_t, capacity> sparse_to_dense;
        
};

