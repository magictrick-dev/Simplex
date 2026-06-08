#pragma once
#include <utils/defs.hpp>
#include <type_traits>

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
            constexpr array() = default;
            inline ~array() = default;

            /// @brief Constructs the array directly from a list of elements,
            /// enabling CTAD (e.g. spx::array{1, 2, 3}). Guarded so it never
            /// shadows the copy/move constructors with a single array argument.
            template <typename... args_t, typename = std::enable_if_t<
                !(sizeof...(args_t) == 1 &&
                  (std::is_same_v<array, std::decay_t<args_t>> && ...))>>
            constexpr array(args_t&&... args)
                : elements{ std::forward<args_t>(args)... } {}

            constexpr type_t& get(size_t index)                    { return elements[index];           }
            constexpr type_t& operator[](size_t index)             { return this->get(index);          }
            constexpr type_t* begin()                              { return elements;                  }
            constexpr type_t* end()                                { return elements + capacity;       }
            constexpr size_t size() const                          { return capacity;                  }
            constexpr const type_t& get(size_t index) const        { return elements[index];           }
            constexpr const type_t& operator[](size_t index) const { return this->get(index);          }
            constexpr const type_t* begin() const                  { return elements;                  }
            constexpr const type_t* end() const                    { return elements + capacity;       }

        private:
            type_t elements[capacity];

    };

    /// @brief CTAD guide for array, deduces both element type and count from
    /// the braced initializer list (e.g. spx::array{1, 2, 3} -> array<int, 3>).
    template <typename type_t, typename... rest_t>
    array(type_t, rest_t...) -> array<std::decay_t<type_t>, 1 + sizeof...(rest_t)>;

}
