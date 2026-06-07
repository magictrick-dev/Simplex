#pragma once
#include <utils/defs.hpp>
#include <utility>

namespace spx
{

    /// @brief A stack-storage fixed-but-dynamic array.
    /// @tparam type_t The type of elements.
    /// @tparam capacity The capacity of elements.
    ///
    /// Unlike an array, a static_array only initializes elements when elements are emplaced
    /// or pushed into the array, so you can have non-trivially constructable elements
    /// inside the array. It is functionally more efficient than a vector if you know you
    /// have an upper-bound on the elements.
    template <typename type_t, size_t capacity>
    class static_array
    {

        public:
            inline static_array() = default;

            inline static_array(size_t initial_count)
            {

                SIMPLEX_ASSERT(initial_count <= capacity);

                // NOTE(Chris): Mirror std::vector(n) -- size becomes initial_count and each
                //              element is value-initialized (zeroed for trivial types).
                for (size_t i = 0; i < initial_count; ++i)
                {
                    type_t *current = this->get_ptr(i);
                    new (current) type_t();
                }

                this->count = initial_count;

            }

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

            inline ~static_array()
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

            inline type_t& back()
            { 
                SIMPLEX_ASSERT(count > 0);
                return this->get(count - 1);
            }

            inline const type_t& back() const
            { 
                SIMPLEX_ASSERT(count > 0);
                return this->get(count - 1);
            }

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
            inline type_t* get_ptr(size_t index) { return (type_t*)(&buffer[index * sizeof(type_t)]); }
            inline const type_t* get_ptr(size_t index) const { return (type_t*)(&buffer[index * sizeof(type_t)]); }

        private:
            alignas(type_t) std::byte buffer[capacity * sizeof(type_t)];
            size_t count = 0;

    };

}