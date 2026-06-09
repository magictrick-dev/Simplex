#pragma once
#include <utils/defs.hpp>
#include <utility>
#include <cstddef>
#include <ostream>

namespace spx
{

    /// @brief A non-owning view over a contiguous sequence of code units, equivalent to
    ///        std::basic_string_view.
    /// @tparam char_t The code-unit type (char, char16_t, char32_t).
    ///
    /// A string_view holds a pointer + length and owns nothing; the referenced storage must
    /// outlive the view. Like the owning strings it is a view of *code units*, not characters,
    /// so size() and operator[] count code units.
    ///
    /// It is implicitly constructible from any owning string that exposes data() and size()
    /// (dynamic_string and static_string both qualify), mirroring std::string -> std::string_view.
    /// To build an owning string back from a view, use the owning string's (data, length)
    /// constructor: e.g. dynamic_string<char>(view.data(), view.size()).
    ///
    /// NOTE: Unlike a null-terminated string, a view is not guaranteed to be null-terminated,
    ///       so there is no c_str(). After remove_prefix / remove_suffix / substr the view may
    ///       reference an interior slice. Use data() + size() for any C-API hand-off, and only
    ///       when you know the underlying buffer is terminated at the view's end.
    template <typename char_t>
    class string_view
    {

        public:
            static constexpr size_t npos = (size_t)-1;

            inline constexpr string_view() : buffer(NULL), count(0) { }
            inline constexpr string_view(const char_t *cstr) : buffer(cstr), count(str_length(cstr)) { }
            inline constexpr string_view(const char_t *data, size_t length) : buffer(data), count(length) { }

            // Implicit view over any owning string exposing data() (convertible to const char_t*)
            // and size(). Covers dynamic_string<char_t> and static_string<char_t, N>.
            template <typename string_t,
                      typename = decltype(static_cast<const char_t*>(std::declval<const string_t&>().data())),
                      typename = decltype(std::declval<const string_t&>().size())>
            inline string_view(const string_t &owner) : buffer(owner.data()), count(owner.size()) { }

            inline constexpr const char_t& get(size_t index) const        { return buffer[index];                                 }
            inline constexpr const char_t& operator[](size_t index) const { return buffer[index];                                 }
            inline constexpr const char_t& front() const                 { SIMPLEX_ASSERT(count > 0); return buffer[0];          }
            inline constexpr const char_t& back() const                  { SIMPLEX_ASSERT(count > 0); return buffer[count - 1];  }
            inline constexpr const char_t* data() const                  { return buffer;                                        }
            inline constexpr const char_t* begin() const                 { return buffer;                                        }
            inline constexpr const char_t* end() const                   { return buffer + count;                                }
            inline constexpr size_t size() const                         { return this->count;                                   }
            inline constexpr size_t length() const                       { return this->count;                                   }
            inline constexpr bool empty() const                          { return this->count == 0;                              }

            inline constexpr void
            remove_prefix(size_t n)
            {
                SIMPLEX_ASSERT(n <= this->count);
                this->buffer += n;
                this->count -= n;
            }

            inline constexpr void
            remove_suffix(size_t n)
            {
                SIMPLEX_ASSERT(n <= this->count);
                this->count -= n;
            }

            inline constexpr string_view<char_t>
            substr(size_t pos, size_t n = npos) const
            {
                SIMPLEX_ASSERT(pos <= this->count);
                size_t remaining = this->count - pos;
                size_t length = (n < remaining) ? n : remaining;
                return string_view<char_t>(this->buffer + pos, length);
            }

            // Returns < 0, 0, > 0 like std::string_view::compare (lexicographic over code units).
            inline constexpr int
            compare(const string_view<char_t> &other) const
            {
                size_t shared = (this->count < other.count) ? this->count : other.count;
                for (size_t i = 0; i < shared; ++i)
                {
                    if (this->buffer[i] < other.buffer[i]) return -1;
                    if (this->buffer[i] > other.buffer[i]) return  1;
                }
                if (this->count < other.count) return -1;
                if (this->count > other.count) return  1;
                return 0;
            }

            inline constexpr bool
            starts_with(const string_view<char_t> &prefix) const
            {
                if (prefix.count > this->count) return false;
                for (size_t i = 0; i < prefix.count; ++i)
                    if (this->buffer[i] != prefix.buffer[i]) return false;
                return true;
            }

            inline constexpr bool
            ends_with(const string_view<char_t> &suffix) const
            {
                if (suffix.count > this->count) return false;
                const size_t offset = this->count - suffix.count;
                for (size_t i = 0; i < suffix.count; ++i)
                    if (this->buffer[offset + i] != suffix.buffer[i]) return false;
                return true;
            }

            inline constexpr bool starts_with(char_t value) const { return this->count > 0 && this->buffer[0] == value;             }
            inline constexpr bool ends_with(char_t value) const   { return this->count > 0 && this->buffer[this->count - 1] == value; }

            inline constexpr size_t
            find(char_t value, size_t pos = 0) const
            {
                for (size_t i = pos; i < this->count; ++i)
                    if (this->buffer[i] == value) return i;
                return npos;
            }

            inline constexpr size_t
            find(const string_view<char_t> &needle, size_t pos = 0) const
            {
                if (needle.count == 0) return (pos <= this->count) ? pos : npos;
                if (needle.count > this->count) return npos;

                const size_t last = this->count - needle.count;
                for (size_t i = pos; i <= last; ++i)
                {
                    size_t j = 0;
                    for (; j < needle.count; ++j)
                        if (this->buffer[i + j] != needle.buffer[j]) break;
                    if (j == needle.count) return i;
                }
                return npos;
            }

            inline constexpr bool operator==(const string_view<char_t> &other) const { return this->compare(other) == 0; }
            inline constexpr bool operator!=(const string_view<char_t> &other) const { return this->compare(other) != 0; }
            inline constexpr bool operator< (const string_view<char_t> &other) const { return this->compare(other) <  0; }
            inline constexpr bool operator> (const string_view<char_t> &other) const { return this->compare(other) >  0; }
            inline constexpr bool operator<=(const string_view<char_t> &other) const { return this->compare(other) <= 0; }
            inline constexpr bool operator>=(const string_view<char_t> &other) const { return this->compare(other) >= 0; }

            /// @brief Computes the length of a null-terminated code-unit string.
            static inline constexpr size_t
            str_length(const char_t *cstr)
            {
                if (cstr == NULL) return 0;
                size_t length = 0;
                while (cstr[length] != char_t(0)) ++length;
                return length;
            }

        private:
            const char_t *buffer;
            size_t count;

    };

    /// @brief Streams the view's code units to an ostream of matching char type.
    /// The view is not guaranteed to be null-terminated, so size() bounds the write.
    template <typename char_t, typename traits_t>
    inline std::basic_ostream<char_t, traits_t>&
    operator<<(std::basic_ostream<char_t, traits_t>& os, const string_view<char_t>& view)
    {
        return os.write(view.data(), static_cast<std::streamsize>(view.size()));
    }

}
