#pragma once
#include <utils/defs.hpp>
#include <cstddef>

namespace spx
{

    /// @brief A stack-storage fixed-capacity string of code units.
    /// @tparam char_t The code-unit type (char, char16_t, char32_t).
    /// @tparam capacity The maximum number of code units (excluding the null terminator).
    ///
    /// The static_string is backed by an in-place buffer of capacity + 1 code units, so it
    /// performs no heap allocation. Like dynamic_string it is a container of *code units*, not
    /// characters: size() and operator[] count code units, so for any encoding wider than ASCII
    /// they do not correspond to user-perceived characters. The buffer is always kept
    /// null-terminated (buffer[count] == char_t(0)), so c_str() is always safe to hand to a C
    /// API. On Windows, static_string<char16_t, N> maps directly to LPCWSTR via a
    /// reinterpret_cast since sizeof(wchar_t) == sizeof(char16_t).
    template <typename char_t, size_t capacity>
    class static_string
    {

        public:
            inline static_string()                              { this->terminate(); }
            inline virtual ~static_string()                     = default;

            inline static_string(const char_t *cstr)
            {
                const size_t length = str_length(cstr);
                SIMPLEX_ASSERT(length <= capacity);
                for (size_t i = 0; i < length; ++i) this->buffer[i] = cstr[i];
                this->count = length;
                this->terminate();
            }

            inline static_string(const char_t *data, size_t length)
            {
                SIMPLEX_ASSERT(length <= capacity);
                for (size_t i = 0; i < length; ++i) this->buffer[i] = data[i];
                this->count = length;
                this->terminate();
            }

            inline static_string(const static_string<char_t, capacity>& other)
            {
                for (size_t i = 0; i < other.count; ++i) this->buffer[i] = other.buffer[i];
                this->count = other.count;
                this->terminate();
            }

            inline static_string(static_string<char_t, capacity>&& other)
            {
                for (size_t i = 0; i < other.count; ++i) this->buffer[i] = other.buffer[i];
                this->count = other.count;
                this->terminate();
                other.count = 0;
                other.terminate();
            }

            inline static_string<char_t, capacity>&
            operator=(const static_string<char_t, capacity>& other)
            {
                if (this == &other) return *this;
                for (size_t i = 0; i < other.count; ++i) this->buffer[i] = other.buffer[i];
                this->count = other.count;
                this->terminate();
                return *this;
            }

            inline static_string<char_t, capacity>&
            operator=(static_string<char_t, capacity>&& other)
            {
                if (this == &other) return *this;
                for (size_t i = 0; i < other.count; ++i) this->buffer[i] = other.buffer[i];
                this->count = other.count;
                this->terminate();
                other.count = 0;
                other.terminate();
                return *this;
            }

            inline static_string<char_t, capacity>&
            operator=(const char_t *cstr)
            {
                this->clear();
                this->append(cstr);
                return *this;
            }

            inline char_t& get(size_t index)                    { return buffer[index];                                 }
            inline char_t& operator[](size_t index)             { return this->get(index);                              }
            inline char_t& front()                              { SIMPLEX_ASSERT(count > 0); return buffer[0];          }
            inline char_t& back()                               { SIMPLEX_ASSERT(count > 0); return buffer[count - 1];  }
            inline char_t* data()                               { return buffer;                                        }
            inline char_t* begin()                              { return buffer;                                        }
            inline char_t* end()                                { return buffer + count;                                }
            inline const char_t* c_str() const                  { return buffer;                                        }
            inline const char_t* data() const                   { return buffer;                                        }
            inline const char_t& get(size_t index) const        { return buffer[index];                                 }
            inline const char_t& operator[](size_t index) const { return this->get(index);                              }
            inline const char_t& front() const                  { SIMPLEX_ASSERT(count > 0); return buffer[0];          }
            inline const char_t& back() const                   { SIMPLEX_ASSERT(count > 0); return buffer[count - 1];  }
            inline const char_t* begin() const                  { return buffer;                                        }
            inline const char_t* end() const                    { return buffer + count;                                }
            inline size_t size() const                          { return this->count;                                   }
            inline size_t length() const                        { return this->count;                                   }
            inline size_t max_size() const                      { return capacity;                                      }
            inline bool empty() const                           { return this->count == 0;                              }
            inline bool full() const                            { return this->count == capacity;                       }

            inline void
            clear()
            {
                this->count = 0;
                this->terminate();
            }

            inline void
            push_back(char_t value)
            {
                SIMPLEX_ASSERT(this->count < capacity);
                this->buffer[this->count] = value;
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

            inline static_string<char_t, capacity>&
            append(const char_t *data, size_t length)
            {
                SIMPLEX_ASSERT(this->count + length <= capacity);
                for (size_t i = 0; i < length; ++i) this->buffer[this->count + i] = data[i];
                this->count += length;
                this->terminate();
                return *this;
            }

            inline static_string<char_t, capacity>&
            append(const char_t *cstr)
            {
                return this->append(cstr, str_length(cstr));
            }

            inline static_string<char_t, capacity>&
            append(const static_string<char_t, capacity>& other)
            {
                return this->append(other.buffer, other.count);
            }

            inline static_string<char_t, capacity>& operator+=(const char_t *cstr)                          { return this->append(cstr);            }
            inline static_string<char_t, capacity>& operator+=(const static_string<char_t, capacity>& o)    { return this->append(o);               }
            inline static_string<char_t, capacity>& operator+=(char_t value)                                { this->push_back(value); return *this; }

            inline bool
            operator==(const static_string<char_t, capacity>& other) const
            {
                if (this->count != other.count) return false;
                for (size_t i = 0; i < this->count; ++i)
                    if (this->buffer[i] != other.buffer[i]) return false;
                return true;
            }

            inline bool
            operator==(const char_t *cstr) const
            {
                size_t i = 0;
                for (; i < this->count; ++i)
                {
                    if (cstr[i] == char_t(0)) return false;
                    if (this->buffer[i] != cstr[i]) return false;
                }
                return cstr[i] == char_t(0);
            }

            inline bool operator!=(const static_string<char_t, capacity>& other) const { return !(*this == other); }
            inline bool operator!=(const char_t *cstr) const                           { return !(*this == cstr);  }

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

            // Maintains the trailing null terminator invariant. count <= capacity, so
            // buffer[count] is always a valid slot within the capacity + 1 storage.
            inline void
            terminate()
            {
                this->buffer[this->count] = char_t(0);
            }

        private:
            char_t buffer[capacity + 1];
            size_t count = 0;

    };

}
