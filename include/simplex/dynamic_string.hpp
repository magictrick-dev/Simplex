#pragma once
#include <utils/defs.hpp>
#include <utils/system/memory_alloc.hpp>
#include <simplex/string_view.hpp>
#include <simplex/static_string.hpp>
#include <utility>
#include <cstddef>
#include <ostream>

namespace spx
{

    /// @brief A dynamically resizing string of code units with STL-like behaviors.
    /// @tparam char_t The code-unit type (char, char16_t, char32_t).
    ///
    /// The dynamic_string is a container of *code units*, not characters: size() and
    /// operator[] count code units, so for any encoding wider than ASCII they do not
    /// correspond to user-perceived characters. Encoding-aware operations (UTF-8 <-> UTF-16
    /// conversion, code-point iteration) belong in free functions layered on top of this
    /// container, not in the container itself.
    ///
    /// The backing buffer grows in multiples of two and does not automatically shrink. It is
    /// always kept null-terminated (elements[count] == char_t(0)), so c_str() is always safe
    /// to hand to a C API. On Windows, dynamic_string<char16_t> data maps directly to LPCWSTR
    /// via a reinterpret_cast since sizeof(wchar_t) == sizeof(char16_t); wchar_t is treated as
    /// an interop cast target, never a storage type.
    ///
    /// The user is responsible for shrink_to_fit() calls as the string drains, as the backing
    /// buffer is never released until destruction or an explicit shrink.
    template <typename char_t>
    class dynamic_string
    {

        public:
            inline dynamic_string() : elements(NULL), reserved(0), count(0) { this->increase_reserves(); this->terminate(); }
            inline virtual ~dynamic_string()                               { this->release_memory();                       }

            inline dynamic_string(const char_t *cstr) : elements(NULL), reserved(0), count(0)
            {
                const size_t length = str_length(cstr);
                this->ensure_capacity(length);
                for (size_t i = 0; i < length; ++i) this->elements[i] = cstr[i];
                this->count = length;
                this->terminate();
            }

            inline dynamic_string(const char_t *data, size_t length) : elements(NULL), reserved(0), count(0)
            {
                this->ensure_capacity(length);
                for (size_t i = 0; i < length; ++i) this->elements[i] = data[i];
                this->count = length;
                this->terminate();
            }

            inline dynamic_string(const string_view<char_t>& view) : elements(NULL), reserved(0), count(0)
            {
                const size_t length = view.size();
                this->ensure_capacity(length);
                for (size_t i = 0; i < length; ++i) this->elements[i] = view[i];
                this->count = length;
                this->terminate();
            }

            template <size_t capacity>
            inline dynamic_string(const static_string<char_t, capacity>& other) : elements(NULL), reserved(0), count(0)
            {
                const size_t length = other.size();
                this->ensure_capacity(length);
                for (size_t i = 0; i < length; ++i) this->elements[i] = other[i];
                this->count = length;
                this->terminate();
            }

            inline dynamic_string(const dynamic_string<char_t>& other) : elements(NULL), reserved(0), count(0)
            {
                this->set_reserves(other.reserved);
                for (size_t i = 0; i < other.count; ++i) this->elements[i] = other.elements[i];
                this->count = other.count;
                this->terminate();
            }

            inline dynamic_string(dynamic_string<char_t>&& other)
            {
                this->reserved = other.reserved;
                this->elements = other.elements;
                this->count = other.count;

                other.reserved = 0;
                other.count = 0;
                other.elements = NULL;
            }

            inline dynamic_string<char_t>&
            operator=(const dynamic_string<char_t>& other)
            {
                if (this == &other) return *this;

                this->release_memory();
                this->set_reserves(other.reserved);
                for (size_t i = 0; i < other.count; ++i) this->elements[i] = other.elements[i];
                this->count = other.count;
                this->terminate();

                return *this;
            }

            inline dynamic_string<char_t>&
            operator=(dynamic_string<char_t>&& other)
            {
                if (this == &other) return *this;

                this->release_memory();

                this->reserved = other.reserved;
                this->elements = other.elements;
                this->count = other.count;

                other.reserved = 0;
                other.count = 0;
                other.elements = NULL;

                return *this;
            }

            inline dynamic_string<char_t>&
            operator=(const char_t *cstr)
            {
                this->clear();
                this->append(cstr);
                return *this;
            }

            inline char_t& get(size_t index)                    { return elements[index];                               }
            inline char_t& operator[](size_t index)             { return this->get(index);                              }
            inline char_t& front()                              { SIMPLEX_ASSERT(count > 0); return elements[0];        }
            inline char_t& back()                               { SIMPLEX_ASSERT(count > 0); return elements[count - 1];}
            inline char_t* data()                               { return elements;                                      }
            inline char_t* begin()                              { return elements;                                      }
            inline char_t* end()                                { return elements + count;                              }
            inline const char_t* c_str() const                  { return elements;                                      }
            inline const char_t* data() const                   { return elements;                                      }
            inline const char_t& get(size_t index) const        { return elements[index];                               }
            inline const char_t& operator[](size_t index) const { return this->get(index);                              }
            inline const char_t& front() const                  { SIMPLEX_ASSERT(count > 0); return elements[0];        }
            inline const char_t& back() const                   { SIMPLEX_ASSERT(count > 0); return elements[count - 1];}
            inline const char_t* begin() const                  { return elements;                                      }
            inline const char_t* end() const                    { return elements + count;                              }
            inline size_t size() const                          { return this->count;                                   }
            inline size_t length() const                        { return this->count;                                   }
            inline size_t capacity() const                      { return this->reserved;                                }
            inline bool empty() const                           { return this->count == 0;                              }

            inline void
            clear()
            {
                this->count = 0;
                this->terminate();
            }

            inline void
            reserve(size_t length)
            {
                this->ensure_capacity(length);
            }

            inline void
            push_back(char_t value)
            {
                this->ensure_capacity(this->count + 1);
                this->elements[this->count] = value;
                this->count++;
                this->terminate();
            }

            inline void
            pop_back()
            {
                SIMPLEX_ASSERT(this->count > 0);
                this->count--;
                this->terminate();
            }

            inline dynamic_string<char_t>&
            append(const char_t *data, size_t length)
            {
                this->ensure_capacity(this->count + length);
                for (size_t i = 0; i < length; ++i) this->elements[this->count + i] = data[i];
                this->count += length;
                this->terminate();
                return *this;
            }

            inline dynamic_string<char_t>&
            append(const char_t *cstr)
            {
                return this->append(cstr, str_length(cstr));
            }

            inline dynamic_string<char_t>&
            append(const dynamic_string<char_t>& other)
            {
                return this->append(other.elements, other.count);
            }

            inline dynamic_string<char_t>&
            append(const string_view<char_t>& view)
            {
                return this->append(view.data(), view.size());
            }

            template <size_t capacity>
            inline dynamic_string<char_t>&
            append(const static_string<char_t, capacity>& other)
            {
                return this->append(other.data(), other.size());
            }

            inline dynamic_string<char_t>& operator+=(const char_t *cstr)                { return this->append(cstr);    }
            inline dynamic_string<char_t>& operator+=(const dynamic_string<char_t>& o)   { return this->append(o);       }
            inline dynamic_string<char_t>& operator+=(const string_view<char_t>& view)   { return this->append(view);    }
            inline dynamic_string<char_t>& operator+=(char_t value)                      { this->push_back(value); return *this; }

            template <size_t capacity>
            inline dynamic_string<char_t>& operator+=(const static_string<char_t, capacity>& o) { return this->append(o); }

            inline bool
            operator==(const dynamic_string<char_t>& other) const
            {
                if (this->count != other.count) return false;
                for (size_t i = 0; i < this->count; ++i)
                    if (this->elements[i] != other.elements[i]) return false;
                return true;
            }

            inline bool
            operator==(const char_t *cstr) const
            {
                size_t i = 0;
                for (; i < this->count; ++i)
                {
                    if (cstr[i] == char_t(0)) return false;
                    if (this->elements[i] != cstr[i]) return false;
                }
                return cstr[i] == char_t(0);
            }

            inline bool operator!=(const dynamic_string<char_t>& other) const { return !(*this == other); }
            inline bool operator!=(const char_t *cstr) const                  { return !(*this == cstr);  }

            inline void
            shrink_to_fit()
            {
                size_t new_capacity = get_minimum_reserved();
                while (new_capacity < this->count + 1) new_capacity *= 2;

                if (new_capacity == this->reserved) return;

                char_t *new_elements = (char_t*)simplex_memory_alloc(new_capacity * sizeof(char_t));
                for (size_t i = 0; i < this->count; ++i) new_elements[i] = this->elements[i];

                simplex_memory_free(this->elements);
                this->elements = new_elements;
                this->reserved = new_capacity;
                this->terminate();
            }

            inline constexpr size_t
            get_minimum_reserved() const
            {
                if constexpr (sizeof(char_t) < 4)       return 64;
                else if constexpr (sizeof(char_t) < 8)  return 32;
                else if constexpr (sizeof(char_t) < 16) return 16;
                else if constexpr (sizeof(char_t) < 32) return 8;
                else if constexpr (sizeof(char_t) < 64) return 4;
                else                                    return 1;
            }

            /// @brief Computes the length of a null-terminated code-unit string.
            static inline size_t
            str_length(const char_t *cstr)
            {
                if (cstr == NULL) return 0;
                size_t length = 0;
                while (cstr[length] != char_t(0)) ++length;
                return length;
            }

        private:

            // Maintains the trailing null terminator invariant. Requires reserved > count.
            inline void
            terminate()
            {
                this->elements[this->count] = char_t(0);
            }

            // Ensures the backing buffer can hold at least `length` code units plus the null
            // terminator, growing geometrically if necessary.
            inline void
            ensure_capacity(size_t length)
            {
                if (this->reserved >= length + 1) return;

                size_t new_capacity = (this->reserved == 0) ? get_minimum_reserved() : this->reserved;
                while (new_capacity < length + 1) new_capacity *= 2;

                char_t *new_elements = (char_t*)simplex_memory_alloc(new_capacity * sizeof(char_t));
                for (size_t i = 0; i < this->count; ++i) new_elements[i] = this->elements[i];

                if (this->elements != NULL) simplex_memory_free(this->elements);
                this->elements = new_elements;
                this->reserved = new_capacity;
            }

            inline void
            set_reserves(size_t size)
            {
                if (this->elements != NULL) this->release_memory();
                this->elements = (char_t*)simplex_memory_alloc(size * sizeof(char_t));
                this->reserved = size;
            }

            inline void
            increase_reserves()
            {
                if (this->elements == NULL)
                {
                    const size_t minimum = get_minimum_reserved();
                    this->elements = (char_t*)simplex_memory_alloc(minimum * sizeof(char_t));
                    this->reserved = minimum;
                    return;
                }

                size_t new_capacity = this->reserved * 2;
                char_t *new_elements = (char_t*)simplex_memory_alloc(new_capacity * sizeof(char_t));
                for (size_t i = 0; i < this->count; ++i) new_elements[i] = this->elements[i];

                simplex_memory_free(this->elements);
                this->elements = new_elements;
                this->reserved = new_capacity;
            }

            inline void
            release_memory()
            {
                if (this->elements != NULL)
                {
                    simplex_memory_free(this->elements);
                    this->elements = NULL;
                }

                this->reserved = 0;
                this->count = 0;
            }

        private:
            char_t *elements;
            size_t reserved;
            size_t count;

    };

    /// @brief Streams the string's code units to an ostream of matching char type.
    template <typename char_t, typename traits_t>
    inline std::basic_ostream<char_t, traits_t>&
    operator<<(std::basic_ostream<char_t, traits_t>& os, const dynamic_string<char_t>& str)
    {
        return os.write(str.data(), static_cast<std::streamsize>(str.size()));
    }

}