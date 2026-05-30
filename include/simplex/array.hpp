#pragma once
#include <utils/defs.hpp>

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
            inline size_t size() const                          { return capacity;                  }
            inline const type_t& get(size_t index) const        { return elements[index];           }
            inline const type_t& operator[](size_t index) const { return this->get(index);          }
            inline const type_t* begin() const                  { return elements;                  }
            inline const type_t* end() const                    { return elements + capacity;       }

        private:
            type_t elements[capacity];

    };

}
