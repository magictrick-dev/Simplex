#pragma once
#include <utils/defs.hpp>
#include <new>
#include <utility>
#include <iterator>
#include <type_traits>
#include <cstddef>

namespace spx
{

    /// @brief A stack-storage fixed-capacity double-ended queue.
    /// @tparam type_t The type of elements.
    /// @tparam capacity The maximum number of elements.
    ///
    /// The static_deque is backed by an in-place circular (ring) buffer, so it performs no
    /// heap allocation. Like static_array, elements are only constructed when inserted and
    /// destroyed when removed, so non-trivially constructible types are supported. Elements
    /// may be inserted and removed at either end via push_front()/push_back() and
    /// pop_front()/pop_back(), while still offering random access through operator[] / get().
    ///
    /// Logical index i maps to the storage slot (head + i) % capacity.
    template <typename type_t, size_t capacity>
    class static_deque
    {

        public:
            inline static_deque() = default;

            inline static_deque(size_t initial_count)
            {

                SIMPLEX_ASSERT(initial_count <= capacity);

                // NOTE(Chris): Mirror std::deque(n) -- size becomes initial_count and each
                //              element is value-initialized (zeroed for trivial types).
                for (size_t i = 0; i < initial_count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    new (current) type_t();
                }

                this->count = initial_count;
                this->head = 0;

            }

            inline static_deque(const static_deque<type_t, capacity>& other)
            {

                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    new (current) type_t(other.get(i));
                }

                this->count = other.count;
                this->head = 0;

            }

            inline static_deque(static_deque<type_t, capacity>&& other)
            {

                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *previous = other.get_ptr(i);
                    type_t *current = this->get_ptr(i);
                    new (current) type_t(std::move(*previous));
                    previous->~type_t();
                }

                this->count = other.count;
                this->head = 0;
                other.count = 0;
                other.head = 0;

            }

            virtual inline ~static_deque()
            {

                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    current->~type_t();
                }

                this->count = 0;
                this->head = 0;

            }

            inline static_deque<type_t, capacity>&
            operator=(const static_deque<type_t, capacity>& other)
            {

                if (this == &other) return *this;

                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    current->~type_t();
                }

                this->count = 0;
                this->head = 0;

                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    new (current) type_t(other.get(i));
                }

                this->count = other.count;

                return *this;

            }

            inline static_deque<type_t, capacity>&
            operator=(static_deque<type_t, capacity>&& other)
            {

                if (this == &other) return *this;

                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    current->~type_t();
                }

                this->count = 0;
                this->head = 0;

                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *previous = other.get_ptr(i);
                    type_t *current = this->get_ptr(i);
                    new (current) type_t(std::move(*previous));
                    previous->~type_t();
                }

                this->count = other.count;
                other.count = 0;
                other.head = 0;

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

                    inline basic_iterator(value_t *elements, size_t head, size_t pos)
                        : elements(elements), head(head), pos(pos) {}

                    inline value_t& operator*() const           { return elements[(head + pos) % capacity];         }
                    inline value_t* operator->() const          { return elements + ((head + pos) % capacity);      }
                    inline value_t& operator[](size_t n) const  { return elements[(head + pos + n) % capacity];     }

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
                    size_t head;
                    size_t pos;

            };

            typedef basic_iterator<type_t>          iterator;
            typedef basic_iterator<const type_t>    const_iterator;

            inline iterator begin()              { return iterator((type_t*)buffer, head, 0);                   }
            inline iterator end()                { return iterator((type_t*)buffer, head, count);               }
            inline const_iterator begin() const  { return const_iterator((const type_t*)buffer, head, 0);       }
            inline const_iterator end() const    { return const_iterator((const type_t*)buffer, head, count);   }
            inline const_iterator cbegin() const { return const_iterator((const type_t*)buffer, head, 0);       }
            inline const_iterator cend() const   { return const_iterator((const type_t*)buffer, head, count);   }

            inline type_t& front()                              { SIMPLEX_ASSERT(count > 0); return *get_ptr(0);            }
            inline type_t& back()                               { SIMPLEX_ASSERT(count > 0); return *get_ptr(count - 1);    }
            inline type_t& get(size_t index)                    { return *get_ptr(index);                                  }
            inline type_t& operator[](size_t index)             { return this->get(index);                                 }
            inline size_t size() const                          { return this->count;                                      }
            inline bool empty() const                           { return this->count == 0;                                 }
            inline bool full() const                            { return this->count == capacity;                          }
            inline const type_t& front() const                  { SIMPLEX_ASSERT(count > 0); return *get_ptr(0);            }
            inline const type_t& back() const                   { SIMPLEX_ASSERT(count > 0); return *get_ptr(count - 1);    }
            inline const type_t& get(size_t index) const        { return *get_ptr(index);                                  }
            inline const type_t& operator[](size_t index) const { return this->get(index);                                 }

            template <typename... Args> inline void
            emplace_back(Args&&... args)
            {

                SIMPLEX_ASSERT(count < capacity);

                type_t *current = this->get_ptr(this->count);
                new (current) type_t(std::forward<Args>(args)...);
                this->count++;

            }

            template <typename... Args> inline void
            emplace_front(Args&&... args)
            {

                SIMPLEX_ASSERT(count < capacity);

                this->head = (this->head + capacity - 1) % capacity;
                type_t *current = this->get_ptr(0);
                new (current) type_t(std::forward<Args>(args)...);
                this->count++;

            }

            inline void
            push_back(const type_t &value)
            {

                SIMPLEX_ASSERT(count < capacity);

                type_t *current = this->get_ptr(this->count);
                new (current) type_t(value);
                this->count++;

            }

            inline void
            push_back(type_t &&value)
            {

                SIMPLEX_ASSERT(count < capacity);

                type_t *current = this->get_ptr(this->count);
                new (current) type_t(std::move(value));
                this->count++;

            }

            inline void
            push_front(const type_t &value)
            {

                SIMPLEX_ASSERT(count < capacity);

                this->head = (this->head + capacity - 1) % capacity;
                type_t *current = this->get_ptr(0);
                new (current) type_t(value);
                this->count++;

            }

            inline void
            push_front(type_t &&value)
            {

                SIMPLEX_ASSERT(count < capacity);

                this->head = (this->head + capacity - 1) % capacity;
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

                this->head = (this->head + 1) % capacity;
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

        private:
            inline type_t* get_ptr(size_t index)
            {
                const size_t slot = (this->head + index) % capacity;
                return (type_t*)(&buffer[slot * sizeof(type_t)]);
            }

            inline const type_t* get_ptr(size_t index) const
            {
                const size_t slot = (this->head + index) % capacity;
                return (type_t*)(&buffer[slot * sizeof(type_t)]);
            }

        private:
            alignas(type_t) std::byte buffer[capacity * sizeof(type_t)];
            size_t count = 0;
            size_t head = 0;

    };

}
