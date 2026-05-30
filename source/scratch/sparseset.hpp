#pragma once
#include <utils/defs.hpp>
#include <scratch/entity.hpp>

#include <vector>
#include <array>
#include <utility>
#include <cstddef>
#include <new>

namespace spx
{

    template <typename type_t, size_t capacity>
    class array
    {

        public:
            inline array() = default;
            inline ~array() = default;

            inline type_t& get(size_t index)                    { return elements[index];           }
            inline type_t& operator[](size_t index)             { return this->get(index);          }
            inline type_t* begin()                              { return elements;                  }
            inline type_t* end()                                { return elements + capacity;       }
            inline size_t size() const                          { return capacity;                  }
            inline const type_t& get(size_t index) const        { return elements[index];           }
            inline const type_t& operator[](size_t index) const { return this->get(index);          }
            inline const type_t* begin() const                  { return elements;                  }
            inline const type_t* end() const                    { return elements + capacity;       }

        private:
            type_t elements[capacity];

    };

    template <typename type_t>
    class dynamic_array
    {

        public:
            inline dynamic_array() : elements(NULL), reserved(0), count(0) { this->increase_reserves();    }
            inline virtual ~dynamic_array()                                { this->release_memory();       }

            inline dynamic_array(const dynamic_array<type_t>& other) : elements(NULL), reserved(0), count(0)
            {

                this->set_reserves(other.reserved);
                this->count = other.count;
                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *current = this->elements + i;
                    new (current) type_t(other[i]);
                }

            }

            inline dynamic_array(dynamic_array<type_t>&& other)
            {
                this->reserved = other.reserved;
                this->elements = other.elements;
                this->count = other.count;

                other.reserved = 0;
                other.count = 0;
                other.elements = NULL;
            }

            inline dynamic_array<type_t>& 
            operator=(const dynamic_array<type_t> &other)
            {

                if (this == &other) return *this;

                this->set_reserves(other.reserved);
                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *current = this->elements + i;
                    new (current) type_t(other[i]);
                }

                this->count = other.count;
                return *this;

            }

            inline dynamic_array<type_t>& 
            operator=(dynamic_array<type_t>&& other)
            {

                if (this == &other) return *this;

                this->release_memory();

                this->reserved = other.reserved;
                this->elements = other.elements;
                this->count = other.count;

                other.reserved = 0;
                other.count = 0;
                other.elements = NULL;

                return *this;

            }

            inline type_t& get(size_t index)                    { return elements[index];           }
            inline type_t& operator[](size_t index)             { return this->get(index);          }
            inline type_t* begin()                              { return elements;                  }
            inline type_t* end()                                { return elements + count;          }
            inline size_t size() const                          { return this->count;               }
            inline size_t capacity() const                      { return this->reserved;            }
            inline const type_t& get(size_t index) const        { return elements[index];           }
            inline const type_t& operator[](size_t index) const { return this->get(index);          }
            inline const type_t* begin() const                  { return elements;                  }
            inline const type_t* end() const                    { return elements + count;          }

            template <typename... Args> inline void
            emplace_back(Args&&... args)
            {
                
                if (this->count == this->reserved)
                {
                    this->increase_reserves();
                }

                const size_t index = this->count;

                type_t *current = this->elements + index;
                new (current) type_t(std::forward<Args>(args)...);
                this->count++;
                
            }

            inline void
            push_back(const type_t &value)
            {

                if (this->count == this->reserved)
                {
                    this->increase_reserves();
                }

                const size_t index = this->count;

                type_t *current = this->elements + index;
                new (current) type_t(value);
                this->count++;

            }

            inline void
            push_back(type_t &&value)
            {

                if (this->count == this->reserved)
                {
                    this->increase_reserves();
                }

                const size_t index = this->count;

                type_t *current = this->elements + index;
                new (current) type_t(std::move(value));
                this->count++;

            }

            inline void
            pop_back()
            {

                SIMPLEX_ASSERT(this->count > 0);

                this->count--;
                const size_t index = this->count;
                type_t *current = this->elements + index;
                current->~type_t();

            }

            inline void shrink_to_fit()
            {

                constexpr size_t new_capacity = get_minimum_reserved();
                while (new_capacity < this->count) new_capacity *= 2;

                type_t *new_elements = (type_t*)simplex_memory_alloc(new_capacity * sizeof(type_t));

                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *previous = elements + i;
                    type_t *current = new_elements + i;
                    new (current) type_t(std::move(this->elements[i]));
                    previous->~type_t();
                }

                simplex_memory_free(elements);
                elements = new_elements;
                reserved = new_capacity;

            }

            inline constexpr size_t get_minimum_reserved()
            {
                size_t minimum = 1;
                if constexpr (sizeof(type_t) < 4)       initial_count = 64;
                else if constexpr (sizeof(type_t) < 8)  initial_count = 32;
                else if constexpr (sizeof(type_t) < 16) initial_count = 16;
                else if constexpr (sizeof(type_t) < 32) initial_count = 8;
                else if constexpr (sizeof(type_t) < 64) initial_count = 4;
                else                                    initial_count = 1;
                return minimum;
            }

        private:

            inline void
            set_reserves(size_t size)
            {

                // TODO(Chris): Enforce a power of two here.

                // NOTE(Chris): simplex_memory_alloc currently provides malloc-level alignment.
                //              Over-aligned type_t support needs an aligned allocation path.
                if (elements != nullptr) this->release_memory();
                this->elements = (type_t*)simplex_memory_alloc(size * sizeof(type_t));
                this->reserved = size;


            }

            inline void 
            increase_reserves()
            {

                // NOTE(Chris): Presizing based on the size of the elements for efficiency.
                //              Maybe we template this as an optional feature?
                if (elements == nullptr)
                {

                    this->elements = (type_t*)simplex_memory_alloc(sizeof(type_t) * get_minimum_reserved());
                    reserved = initial_count;
                    return;

                }

                size_t new_capacity = count * 2;
                type_t *new_elements = (type_t*)simplex_memory_alloc(new_capacity * sizeof(type_t));

                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *previous = elements + i;
                    type_t *current = new_elements + i;
                    new (current) type_t(std::move(this->elements[i]));
                    previous->~type_t();
                }

                simplex_memory_free(elements);

                elements = new_elements;
                reserved = new_capacity;

            }

            inline void
            release_memory()
            {

                if (this->elements != NULL)
                {
                    for (size_t i = 0; i < this->count; ++i)
                    {
                        type_t *current_element = elements + i;
                        current_element->~type_t();
                    }
                    simplex_memory_free(this->elements);
                    this->elements = NULL;
                }

            }

        private:
            type_t *elements;
            size_t count;
            size_t reserved;

    };

    template <typename type_t, size_t capacity>
    class static_array
    {

        public:
            inline static_array() = default;

            inline static_array(const static_array<type_t, capacity>& other)
            {

                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    new (current) type_t(other[i]);
                }

                this->count = other.count;
                
            }

            inline static_array(static_array<type_t, capacity>&& other)
            {

                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *previous = other.get_ptr(i);
                    type_t *current = this->get_ptr(i);
                    new (current) type_t(std::move(other[i]));
                    previous->~type_t();
                }

                this->count = other.count;
                other.count = 0;

            }

            virtual inline ~static_array()
            {

                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    current->~type_t();
                }

                this->count = 0;

            }

            inline static_array<type_t, capacity>&
            operator=(const static_array<type_t, capacity>& other)
            {

                if (this == &other) return *this;

                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    current->~type_t();
                }

                this->count = 0;

                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    new (current) type_t(other[i]);
                }

                this->count = other.count;

                return *this;

            }

            inline static_array<type_t, capacity>&
            operator=(static_array<type_t, capacity>&& other)
            {

                if (this == &other) return *this;

                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    current->~type_t();
                }

                this->count = 0;

                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *previous = other.get_ptr(i);
                    type_t *current = this->get_ptr(i);
                    new (current) type_t(std::move(other[i]));
                    previous->~type_t();
                }

                this->count = other.count;
                other.count = 0;

                return *this;

            }

            inline type_t& get(size_t index)                    { return *get_ptr(index);           }
            inline type_t& operator[](size_t index)             { return this->get(index);          }
            inline type_t* begin()                              { return this->get_ptr(0);          }
            inline type_t* end()                                { return this->get_ptr(count);      }
            inline size_t size() const                          { return this->count;               }
            inline const type_t& get(size_t index) const        { return *get_ptr(index);           }
            inline const type_t& operator[](size_t index) const { return this->get(index);          }
            inline const type_t* begin() const                  { return this->get_ptr(0);          }
            inline const type_t* end() const                    { return this->get_ptr(count);      }

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
            push_back(const type_t &value)
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
        spx::static_array<type_t, capacity> elements;
        std::array<entity_t, capacity> dense_to_sparse;
        std::array<int32_t, capacity> sparse_to_dense;
        
};

