#pragma once
#include <utils/defs.hpp>
#include <utils/system/memory_alloc.hpp>
#include <new>
#include <utility>
#include <iterator>
#include <type_traits>
#include <cstddef>

namespace spx
{

    /// @brief A dynamically resizing double-ended queue with STL-like behaviors equivalent to std::deque.
    /// @tparam type_t The type of elements for the deque.
    ///
    /// The dynamic deque is backed by a circular (ring) buffer which grows in multiples of
    /// two and does not automatically shrink. Elements may be inserted and removed at either
    /// end in amortized constant time via push_front()/push_back() and pop_front()/pop_back(),
    /// while still offering random access through operator[] / get(). Because storage is a ring
    /// buffer, the logical front of the deque does not necessarily map to index zero of the
    /// backing allocation; logical index i lives at (head + i) % reserved.
    ///
    /// The user of this datastructure is responsible for shrink_to_fit() calls when the deque
    /// drains, as the backing buffer is never released until destruction or an explicit shrink.
    template <typename type_t>
    class dynamic_deque
    {

        public:
            inline dynamic_deque() : elements(NULL), reserved(0), count(0), head(0) { this->increase_reserves(); }
            inline virtual ~dynamic_deque()                                         { this->release_memory();    }

            inline dynamic_deque(size_t reserve_count) : elements(NULL), reserved(0), count(0), head(0)
            {

                size_t target = round_up_power_of_two(reserve_count);
                if (target == 0) target = get_minimum_reserved();
                this->set_reserves(target);

                // NOTE(Chris): Mirror std::deque(n) -- size becomes reserve_count and each
                //              element is value-initialized (zeroed for trivial types).
                for (size_t i = 0; i < reserve_count; ++i)
                {
                    type_t *current = this->elements + i;
                    new (current) type_t();
                }

                this->count = reserve_count;
                this->head = 0;

            }

            inline dynamic_deque(const dynamic_deque<type_t>& other) : elements(NULL), reserved(0), count(0), head(0)
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

            inline dynamic_deque(dynamic_deque<type_t>&& other)
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

            inline dynamic_deque<type_t>&
            operator=(const dynamic_deque<type_t>& other)
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

            inline dynamic_deque<type_t>&
            operator=(dynamic_deque<type_t>&& other)
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

            // NOTE(Chris): The backing store is a ring buffer, so a plain pointer cannot walk
            //              the elements in logical order once they wrap. This iterator carries
            //              the logical position and resolves the physical slot on dereference.
            template <typename value_t>
            class basic_iterator
            {

                public:
                    using iterator_category = std::random_access_iterator_tag;
                    using value_type        = std::remove_cv_t<value_t>;
                    using difference_type   = ptrdiff_t;
                    using pointer           = value_t*;
                    using reference         = value_t&;

                    inline basic_iterator(value_t *elements, size_t modulus, size_t head, size_t pos)
                        : elements(elements), modulus(modulus), head(head), pos(pos) {}

                    inline value_t& operator*() const           { return elements[(head + pos) % modulus];          }
                    inline value_t* operator->() const          { return elements + ((head + pos) % modulus);       }
                    inline value_t& operator[](size_t n) const  { return elements[(head + pos + n) % modulus];      }

                    inline basic_iterator& operator++()         { ++pos; return *this;                              }
                    inline basic_iterator  operator++(int)      { basic_iterator t = *this; ++pos; return t;        }
                    inline basic_iterator& operator--()         { --pos; return *this;                              }
                    inline basic_iterator  operator--(int)      { basic_iterator t = *this; --pos; return t;        }

                    inline basic_iterator& operator+=(size_t n) { pos += n; return *this;                           }
                    inline basic_iterator& operator-=(size_t n) { pos -= n; return *this;                           }
                    inline basic_iterator  operator+(size_t n) const { basic_iterator t = *this; t.pos += n; return t; }
                    inline basic_iterator  operator-(size_t n) const { basic_iterator t = *this; t.pos -= n; return t; }
                    inline ptrdiff_t operator-(const basic_iterator &o) const { return (ptrdiff_t)pos - (ptrdiff_t)o.pos; }

                    inline bool operator==(const basic_iterator &o) const { return pos == o.pos; }
                    inline bool operator!=(const basic_iterator &o) const { return pos != o.pos; }
                    inline bool operator<(const basic_iterator &o) const  { return pos <  o.pos; }
                    inline bool operator>(const basic_iterator &o) const  { return pos >  o.pos; }
                    inline bool operator<=(const basic_iterator &o) const { return pos <= o.pos; }
                    inline bool operator>=(const basic_iterator &o) const { return pos >= o.pos; }

                private:
                    value_t *elements;
                    size_t modulus;
                    size_t head;
                    size_t pos;

            };

            typedef basic_iterator<type_t>          iterator;
            typedef basic_iterator<const type_t>    const_iterator;

            inline iterator begin()                             { return iterator(elements, reserved, head, 0);             }
            inline iterator end()                               { return iterator(elements, reserved, head, count);         }
            inline const_iterator begin() const                 { return const_iterator(elements, reserved, head, 0);       }
            inline const_iterator end() const                   { return const_iterator(elements, reserved, head, count);   }
            inline const_iterator cbegin() const                { return const_iterator(elements, reserved, head, 0);       }
            inline const_iterator cend() const                  { return const_iterator(elements, reserved, head, count);   }

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
            emplace_back(Args&&... args)
            {

                if (this->count == this->reserved)
                {
                    this->increase_reserves();
                }

                type_t *current = this->get_ptr(this->count);
                new (current) type_t(std::forward<Args>(args)...);
                this->count++;

            }

            template <typename... Args> inline void
            emplace_front(Args&&... args)
            {

                if (this->count == this->reserved)
                {
                    this->increase_reserves();
                }

                this->head = (this->head + this->reserved - 1) % this->reserved;
                type_t *current = this->get_ptr(0);
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

                type_t *current = this->get_ptr(this->count);
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

                type_t *current = this->get_ptr(this->count);
                new (current) type_t(std::move(value));
                this->count++;

            }

            inline void
            push_front(const type_t &value)
            {

                if (this->count == this->reserved)
                {
                    this->increase_reserves();
                }

                this->head = (this->head + this->reserved - 1) % this->reserved;
                type_t *current = this->get_ptr(0);
                new (current) type_t(value);
                this->count++;

            }

            inline void
            push_front(type_t &&value)
            {

                if (this->count == this->reserved)
                {
                    this->increase_reserves();
                }

                this->head = (this->head + this->reserved - 1) % this->reserved;
                type_t *current = this->get_ptr(0);
                new (current) type_t(std::move(value));
                this->count++;

            }

            inline void
            pop_front()
            {

                SIMPLEX_ASSERT(this->count > 0);

                type_t *current = this->get_ptr(0);
                current->~type_t();

                this->head = (this->head + 1) % this->reserved;
                this->count--;

            }

            inline void
            pop_back()
            {

                SIMPLEX_ASSERT(this->count > 0);

                type_t *current = this->get_ptr(this->count - 1);
                current->~type_t();

                this->count--;

            }

            inline void
            clear()
            {

                // NOTE(Chris): Destroys every element but retains the reserved buffer; use
                //              shrink_to_fit() to reclaim memory afterwards.
                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    current->~type_t();
                }

                this->count = 0;
                this->head = 0;

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

            // NOTE(Chris): Rounds value up to the nearest power of two (31 -> 32, 11 -> 16).
            //              A value that is already a power of two is returned unchanged.
            inline static constexpr size_t
            round_up_power_of_two(size_t value)
            {

                if (value == 0) return 0;

                size_t result = 1;
                while (result < value) result *= 2;
                return result;

            }

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
