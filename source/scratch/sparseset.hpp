#pragma once
#include <utils/defs.hpp>
#include <scratch/entity.hpp>

#include <vector>
#include <array>

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
            inline size_t size() const                          { return this->count;               }
            inline const type_t& get(size_t index) const        { return elements[index];           }
            inline const type_t& operator[](size_t index) const { return this->get(index);          }
            inline const type_t* begin() const                  { return elements;                  }
            inline const type_t* end() const                    { return elements + capacity;       }

        private:
            type_t elements[capacity];

    };

    template <typename type_t>
    class vector
    {

        public:
            inline vector() : elements(NULL), reserved(0), count(0)     { this->increase_capacity(); }
            inline virtual ~vector()                                    { this->release_memory(); }

            inline type_t& get(size_t index)                    { return elements[index];           }
            inline type_t& operator[](size_t index)             { return this->get(index);          }
            inline type_t* begin()                              { return elements;                  }
            inline type_t* end()                                { return elements + count;          }
            inline size_t size() const                          { return this->count;               }
            inline size_t capacity() const                      { return this->capacity;            }
            inline const type_t& get(size_t index) const        { return elements[index];           }
            inline const type_t& operator[](size_t index) const { return this->get(index);          }
            inline const type_t* begin() const                  { return elements;                  }
            inline const type_t* end() const                    { return elements + count;          }

        private:
            inline void 
            increase_capacity()
            {

                // NOTE(Chris): Presizing based on the size of the elements for efficiency.
                //              Maybe we template this as an optional feature?
                if (elements == nullptr)
                {

                    constexpr size_t initial_count = 1;

                    if constexpr (sizeof(type_t) < 4)       size = 64;
                    else if constexpr (sizeof(type_t) < 8)  size = 32;
                    else if constexpr (sizeof(type_t) < 16) size = 16;
                    else if constexpr (sizeof(type_t) < 32) size = 8;
                    else if constexpr (sizeof(type_t) < 64) size = 4;
                    else                                    size = 1;

                    this->elements = simplex_memory_alloc(sizeof(type_t) * initial_count);
                    reserved = initial_count;

                    return;
                }

                size_t new_capacity = count * 2;
                type_t *new_elements = (type_t*)simplex_memory_alloc(new_capacity * sizeof(type_t));
                memcpy(new_elements, elements, count * sizeof(type_t));
                simplex_memory_free(elements);
                elements = new_elements;
                reserved = new_capacity;

            }

            inline void
            decrease_capacity()
            {

                if (this->reserved == 1) return;

                size_t new_capacity = count / 2;
                SIMPLEX_ASSERT(new_capacity >= count);
                type_t *new_elements = (type_t*)simplex_memory_alloc(new_capacity * sizeof(type_t));
                memcpy(new_elements, elements, count *sizeof(type_t));
                simplex_memory_free(elements);
                elements = new_elements;
                reserved = new_capacity;
                
            }

            inline void
            release_memory()
            {
                // NOTE(Chris): Invoke this *only* for the destructor.
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
    class static_vector
    {

        public:
            inline static_vector() = default;
            virtual inline ~static_vector() = default;

            inline type_t& get(size_t index)                    { return *get_ptr(index);           }
            inline type_t& operator[](size_t index)             { return this->get(index);          }
            inline type_t* begin()                              { return this->get_ptr(0);          }
            inline type_t* end()                                { return this->get_ptr(capacity);   }
            inline size_t size() const                          { return this->count;               }
            inline const type_t& get(size_t index) const        { return *get_ptr(index);           }
            inline const type_t& operator[](size_t index) const { return this->get(index);          }
            inline const type_t* begin() const                  { return this->get_ptr(0);          }
            inline const type_t* end() const                    { return this->get_ptr(capacity);   }

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
        spx::static_vector<type_t, capacity> elements;
        std::array<entity_t, capacity> dense_to_sparse;
        std::array<int32_t, capacity> sparse_to_dense;
        
};

