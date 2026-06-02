#pragma once
#include <utils/defs.hpp>
#include <utils/system/memory_alloc.hpp>
#include <new>
#include <utility>
#include <cstddef>

namespace spx
{

    /// @brief A dynamically resizing LIFO stack with STL-like behaviors equivalent to std::stack.
    /// @tparam type_t The type of elements for the stack.
    ///
    /// The dynamic stack grows in multiples of two and does not automatically shrink. Elements
    /// are pushed onto the top via push() / emplace() and removed from the top via pop(). All
    /// activity happens at the tail of the backing allocation, so no element shuffling is
    /// required and the logical top maps directly to index (count - 1).
    ///
    /// The user of this datastructure is responsible for shrink_to_fit() calls as the stack
    /// drains, as the backing buffer is never released until destruction or an explicit shrink.
    template <typename type_t>
    class dynamic_stack
    {

        public:
            inline dynamic_stack() : elements(NULL), reserved(0), count(0) { this->increase_reserves(); }
            inline virtual ~dynamic_stack()                               { this->release_memory();    }

            inline dynamic_stack(const dynamic_stack<type_t>& other) : elements(NULL), reserved(0), count(0)
            {

                this->set_reserves(other.reserved);
                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *current = this->elements + i;
                    new (current) type_t(other.get(i));
                }

                this->count = other.count;

            }

            inline dynamic_stack(dynamic_stack<type_t>&& other)
            {

                this->reserved = other.reserved;
                this->elements = other.elements;
                this->count = other.count;

                other.reserved = 0;
                other.count = 0;
                other.elements = NULL;

            }

            inline dynamic_stack<type_t>&
            operator=(const dynamic_stack<type_t>& other)
            {

                if (this == &other) return *this;

                this->release_memory();
                this->set_reserves(other.reserved);
                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *current = this->elements + i;
                    new (current) type_t(other.get(i));
                }

                this->count = other.count;

                return *this;

            }

            inline dynamic_stack<type_t>&
            operator=(dynamic_stack<type_t>&& other)
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

            inline type_t& top()                                { SIMPLEX_ASSERT(count > 0); return elements[count - 1];    }
            inline type_t& get(size_t index)                    { return elements[index];                                  }
            inline type_t& operator[](size_t index)             { return this->get(index);                                 }
            inline size_t size() const                          { return this->count;                                      }
            inline size_t capacity() const                      { return this->reserved;                                   }
            inline bool empty() const                           { return this->count == 0;                                 }
            inline const type_t& top() const                    { SIMPLEX_ASSERT(count > 0); return elements[count - 1];    }
            inline const type_t& get(size_t index) const        { return elements[index];                                  }
            inline const type_t& operator[](size_t index) const { return this->get(index);                                 }

            template <typename... Args> inline void
            emplace(Args&&... args)
            {

                if (this->count == this->reserved)
                {
                    this->increase_reserves();
                }

                type_t *current = this->elements + this->count;
                new (current) type_t(std::forward<Args>(args)...);
                this->count++;

            }

            inline void
            push(const type_t &value)
            {

                if (this->count == this->reserved)
                {
                    this->increase_reserves();
                }

                type_t *current = this->elements + this->count;
                new (current) type_t(value);
                this->count++;

            }

            inline void
            push(type_t &&value)
            {

                if (this->count == this->reserved)
                {
                    this->increase_reserves();
                }

                type_t *current = this->elements + this->count;
                new (current) type_t(std::move(value));
                this->count++;

            }

            inline void
            pop()
            {

                SIMPLEX_ASSERT(this->count > 0);

                this->count--;
                type_t *current = this->elements + this->count;
                current->~type_t();

            }

            inline void
            shrink_to_fit()
            {

                size_t new_capacity = get_minimum_reserved();
                while (new_capacity < this->count) new_capacity *= 2;

                if (new_capacity == this->reserved) return;

                type_t *new_elements = (type_t*)simplex_memory_alloc(new_capacity * sizeof(type_t));

                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *previous = this->elements + i;
                    type_t *current = new_elements + i;
                    new (current) type_t(std::move(*previous));
                    previous->~type_t();
                }

                simplex_memory_free(this->elements);
                this->elements = new_elements;
                this->reserved = new_capacity;

            }

            inline constexpr size_t
            get_minimum_reserved() const
            {
                if constexpr (sizeof(type_t) < 4)       return 64;
                else if constexpr (sizeof(type_t) < 8)  return 32;
                else if constexpr (sizeof(type_t) < 16) return 16;
                else if constexpr (sizeof(type_t) < 32) return 8;
                else if constexpr (sizeof(type_t) < 64) return 4;
                else                                    return 1;
            }

        private:

            inline void
            set_reserves(size_t size)
            {

                // NOTE(Chris): simplex_memory_alloc currently provides malloc-level alignment.
                //              Over-aligned type_t support needs an aligned allocation path.
                if (this->elements != NULL) this->release_memory();
                this->elements = (type_t*)simplex_memory_alloc(size * sizeof(type_t));
                this->reserved = size;

            }

            inline void
            increase_reserves()
            {

                // NOTE(Chris): Presizing based on the size of the elements for efficiency.
                if (this->elements == NULL)
                {
                    const size_t minimum = get_minimum_reserved();
                    this->elements = (type_t*)simplex_memory_alloc(minimum * sizeof(type_t));
                    this->reserved = minimum;
                    return;
                }

                size_t new_capacity = this->reserved * 2;
                type_t *new_elements = (type_t*)simplex_memory_alloc(new_capacity * sizeof(type_t));

                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *previous = this->elements + i;
                    type_t *current = new_elements + i;
                    new (current) type_t(std::move(*previous));
                    previous->~type_t();
                }

                simplex_memory_free(this->elements);

                this->elements = new_elements;
                this->reserved = new_capacity;

            }

            inline void
            release_memory()
            {

                if (this->elements != NULL)
                {
                    for (size_t i = 0; i < this->count; ++i)
                    {
                        type_t *current_element = this->elements + i;
                        current_element->~type_t();
                    }
                    simplex_memory_free(this->elements);
                    this->elements = NULL;
                }

                this->reserved = 0;
                this->count = 0;

            }

        private:
            type_t *elements;
            size_t reserved;
            size_t count;

    };

}
