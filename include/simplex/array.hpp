#pragma once
#include <utils/defs.hpp>

namespace spx
{

    /// @brief A c-style array wrapper that provides STL-like functionality.
    /// @tparam type_t The type of element for the array.
    /// @tparam capacity The capacity of the array.
    ///
    /// The array class requires trivially constructable objects similar to c-style
    /// arrays. For non-trivially construct objects or for an "in-place" vector, use
    /// static_array instead.
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

}
