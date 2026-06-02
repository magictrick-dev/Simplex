#pragma once
#include <utils/defs.hpp>
#include <new>
#include <utility>
#include <cstddef>

namespace spx
{

    /// @brief A stack-storage fixed-capacity LIFO stack.
    /// @tparam type_t The type of elements.
    /// @tparam capacity The maximum number of elements.
    ///
    /// The static_stack is backed by in-place storage, so it performs no heap allocation.
    /// Like static_array, elements are only constructed when pushed and destroyed when popped,
    /// so non-trivially constructible types are supported. Elements are pushed onto the top via
    /// push() / emplace() and removed from the top via pop(). The logical top maps directly to
    /// index (count - 1).
    template <typename type_t, size_t capacity>
    class static_stack
    {

        public:
            inline static_stack() = default;

            inline static_stack(const static_stack<type_t, capacity>& other)
            {

                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    new (current) type_t(other.get(i));
                }

                this->count = other.count;

            }

            inline static_stack(static_stack<type_t, capacity>&& other)
            {

                for (size_t i = 0; i < other.count; ++i)
                {
                    type_t *previous = other.get_ptr(i);
                    type_t *current = this->get_ptr(i);
                    new (current) type_t(std::move(*previous));
                    previous->~type_t();
                }

                this->count = other.count;
                other.count = 0;

            }

            virtual inline ~static_stack()
            {

                for (size_t i = 0; i < this->count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    current->~type_t();
                }

                this->count = 0;

            }

            inline static_stack<type_t, capacity>&
            operator=(const static_stack<type_t, capacity>& other)
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
                    new (current) type_t(other.get(i));
                }

                this->count = other.count;

                return *this;

            }

            inline static_stack<type_t, capacity>&
            operator=(static_stack<type_t, capacity>&& other)
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
                    new (current) type_t(std::move(*previous));
                    previous->~type_t();
                }

                this->count = other.count;
                other.count = 0;

                return *this;

            }

            inline type_t& top()                                { SIMPLEX_ASSERT(count > 0); return *get_ptr(count - 1);    }
            inline type_t& get(size_t index)                    { return *get_ptr(index);                                  }
            inline type_t& operator[](size_t index)             { return this->get(index);                                 }
            inline size_t size() const                          { return this->count;                                      }
            inline bool empty() const                           { return this->count == 0;                                 }
            inline bool full() const                            { return this->count == capacity;                          }
            inline const type_t& top() const                    { SIMPLEX_ASSERT(count > 0); return *get_ptr(count - 1);    }
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

                this->count--;
                type_t *current = this->get_ptr(this->count);
                current->~type_t();

            }

        private:
            inline type_t* get_ptr(size_t index)
            {
                return (type_t*)(&buffer[index * sizeof(type_t)]);
            }

            inline const type_t* get_ptr(size_t index) const
            {
                return (type_t*)(&buffer[index * sizeof(type_t)]);
            }

        private:
            alignas(type_t) std::byte buffer[capacity * sizeof(type_t)];
            size_t count = 0;

    };

}
