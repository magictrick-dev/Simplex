#pragma once
#include <utils/defs.hpp>
#include <new>
#include <utility>
#include <cstddef>

namespace spx
{

    /// @brief A stack-storage fixed-capacity FIFO queue.
    /// @tparam type_t The type of elements.
    /// @tparam capacity The maximum number of elements.
    ///
    /// The static_queue is backed by an in-place circular (ring) buffer, so it performs no
    /// heap allocation. Like static_array, elements are only constructed when enqueued and
    /// destroyed when dequeued, so non-trivially constructible types are supported. Elements
    /// are enqueued at the back via push() / emplace() and dequeued from the front via pop().
    ///
    /// Logical index i maps to the storage slot (head + i) % capacity.
    template <typename type_t, size_t capacity>
    class static_queue
    {

        public:
            inline static_queue() = default;

            inline static_queue(const static_queue<type_t, capacity>& other)
            {

                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    new (current) type_t(other.get(i));
                }

                this->count = other.count;
                this->head = 0;

            }

            inline static_queue(static_queue<type_t, capacity>&& other)
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

            virtual inline ~static_queue()
            {

                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    current->~type_t();
                }

                this->count = 0;
                this->head = 0;

            }

            inline static_queue<type_t, capacity>&
            operator=(const static_queue<type_t, capacity>& other)
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

            inline static_queue<type_t, capacity>&
            operator=(static_queue<type_t, capacity>&& other)
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
            emplace(Args&&... args)
            {

                SIMPLEX_ASSERT(count < capacity);

                type_t *current = this->get_ptr(this->count);
                new (current) type_t(std::forward<Args>(args)...);
                this->count++;

            }

            inline void
            push(const type_t &value)
            {

                SIMPLEX_ASSERT(count < capacity);

                type_t *current = this->get_ptr(this->count);
                new (current) type_t(value);
                this->count++;

            }

            inline void
            push(type_t &&value)
            {

                SIMPLEX_ASSERT(count < capacity);

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

                this->head = (this->head + 1) % capacity;
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
