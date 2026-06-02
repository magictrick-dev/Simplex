#pragma once
#include <utils/defs.hpp>
#include <utils/system/memory_alloc.hpp>
#include <new>
#include <utility>
#include <cstddef>

namespace spx
{

    /// @brief A dynamically resizing FIFO queue with STL-like behaviors equivalent to std::queue.
    /// @tparam type_t The type of elements for the queue.
    ///
    /// The dynamic queue is backed by a circular (ring) buffer which grows in multiples of
    /// two and does not automatically shrink. Elements are enqueued at the back via push()
    /// / emplace() and dequeued from the front via pop(). Because storage is a ring buffer,
    /// the logical front of the queue does not necessarily map to index zero of the backing
    /// allocation; logical index i lives at (head + i) % reserved.
    ///
    /// The user of this datastructure is responsible for shrink_to_fit() calls when the
    /// queue drains, as the backing buffer is never released until destruction or an
    /// explicit shrink.
    template <typename type_t>
    class dynamic_queue
    {

        public:
            inline dynamic_queue() : elements(NULL), reserved(0), count(0), head(0) { this->increase_reserves(); }
            inline virtual ~dynamic_queue()                                         { this->release_memory();    }

            inline dynamic_queue(const dynamic_queue<type_t>& other) : elements(NULL), reserved(0), count(0), head(0)
            {

                this->set_reserves(other.reserved);
                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *current = this->elements + i;
                    new (current) type_t(other.get(i));
                }

                this->count = other.count;
                this->head = 0;

            }

            inline dynamic_queue(dynamic_queue<type_t>&& other)
            {

                this->reserved = other.reserved;
                this->elements = other.elements;
                this->count = other.count;
                this->head = other.head;

                other.reserved = 0;
                other.count = 0;
                other.head = 0;
                other.elements = NULL;

            }

            inline dynamic_queue<type_t>&
            operator=(const dynamic_queue<type_t>& other)
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
                this->head = 0;

                return *this;

            }

            inline dynamic_queue<type_t>&
            operator=(dynamic_queue<type_t>&& other)
            {

                if (this == &other) return *this;

                this->release_memory();

                this->reserved = other.reserved;
                this->elements = other.elements;
                this->count = other.count;
                this->head = other.head;

                other.reserved = 0;
                other.count = 0;
                other.head = 0;
                other.elements = NULL;

                return *this;

            }

            inline type_t& front()                              { SIMPLEX_ASSERT(count > 0); return *get_ptr(0);            }
            inline type_t& back()                               { SIMPLEX_ASSERT(count > 0); return *get_ptr(count - 1);    }
            inline type_t& get(size_t index)                    { return *get_ptr(index);                                  }
            inline type_t& operator[](size_t index)             { return this->get(index);                                 }
            inline size_t size() const                          { return this->count;                                      }
            inline size_t capacity() const                      { return this->reserved;                                   }
            inline bool empty() const                           { return this->count == 0;                                 }
            inline const type_t& front() const                  { SIMPLEX_ASSERT(count > 0); return *get_ptr(0);            }
            inline const type_t& back() const                   { SIMPLEX_ASSERT(count > 0); return *get_ptr(count - 1);    }
            inline const type_t& get(size_t index) const        { return *get_ptr(index);                                  }
            inline const type_t& operator[](size_t index) const { return this->get(index);                                 }

            template <typename... Args> inline void
            emplace(Args&&... args)
            {

                if (this->count == this->reserved)
                {
                    this->increase_reserves();
                }

                type_t *current = this->get_ptr(this->count);
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

                type_t *current = this->get_ptr(this->count);
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

                type_t *current = this->get_ptr(this->count);
                new (current) type_t(std::move(value));
                this->count++;

            }

            inline void
            pop()
            {

                SIMPLEX_ASSERT(this->count > 0);

                type_t *current = this->get_ptr(0);
                current->~type_t();

                this->head = (this->head + 1) % this->reserved;
                this->count--;

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
                    type_t *previous = this->get_ptr(i);
                    type_t *current = new_elements + i;
                    new (current) type_t(std::move(*previous));
                    previous->~type_t();
                }

                simplex_memory_free(this->elements);
                this->elements = new_elements;
                this->reserved = new_capacity;
                this->head = 0;

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

            inline type_t*
            get_ptr(size_t index)
            {
                return this->elements + ((this->head + index) % this->reserved);
            }

            inline const type_t*
            get_ptr(size_t index) const
            {
                return this->elements + ((this->head + index) % this->reserved);
            }

            inline void
            set_reserves(size_t size)
            {

                // NOTE(Chris): simplex_memory_alloc currently provides malloc-level alignment.
                //              Over-aligned type_t support needs an aligned allocation path.
                if (this->elements != NULL) this->release_memory();
                this->elements = (type_t*)simplex_memory_alloc(size * sizeof(type_t));
                this->reserved = size;
                this->head = 0;

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
                    this->head = 0;
                    return;
                }

                size_t new_capacity = this->reserved * 2;
                type_t *new_elements = (type_t*)simplex_memory_alloc(new_capacity * sizeof(type_t));

                // NOTE(Chris): Linearize the ring into the new buffer so the front maps to
                //              index zero again.
                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *previous = this->get_ptr(i);
                    type_t *current = new_elements + i;
                    new (current) type_t(std::move(*previous));
                    previous->~type_t();
                }

                simplex_memory_free(this->elements);

                this->elements = new_elements;
                this->reserved = new_capacity;
                this->head = 0;

            }

            inline void
            release_memory()
            {

                if (this->elements != NULL)
                {
                    for (size_t i = 0; i < this->count; ++i)
                    {
                        type_t *current_element = this->get_ptr(i);
                        current_element->~type_t();
                    }
                    simplex_memory_free(this->elements);
                    this->elements = NULL;
                }

                this->reserved = 0;
                this->count = 0;
                this->head = 0;

            }

        private:
            type_t *elements;
            size_t reserved;
            size_t count;
            size_t head;

    };

}
