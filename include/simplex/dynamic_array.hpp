#pragma once
#include <utils/defs.hpp>
#include <utils/system/memory_alloc.hpp>
#include <new>
#include <utility>
#include <cstddef>

namespace spx
{

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


};