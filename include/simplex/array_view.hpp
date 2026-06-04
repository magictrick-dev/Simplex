#pragma once
#include <utils/defs.hpp>
#include <utility>
#include <cstddef>

namespace spx
{

    /// @brief A non-owning view over a contiguous sequence of elements, the array analogue of
    ///        string_view (and close kin to std::span).
    /// @tparam type_t The element type.
    ///
    /// An array_view holds a pointer + length and owns nothing; the referenced storage must
    /// outlive the view. It is a read-only window: elements are exposed through const accessors,
    /// so a view never mutates the underlying container.
    ///
    /// It is implicitly constructible from any contiguous container that exposes begin()
    /// (convertible to const type_t*) and size(). array<type_t, N>, dynamic_array<type_t>, and
    /// static_array<type_t, N> all qualify, mirroring the way a container hands itself to a view
    /// without the view including the container headers. To build an owning container back from a
    /// view, feed data() + size() into that container's API.
    ///
    /// NOTE: remove_prefix / remove_suffix / subview / first / last all return or retarget an
    ///       interior slice, so a view is not guaranteed to span a whole allocation. Use data() +
    ///       size() for any hand-off that expects the full backing buffer.
    template <typename type_t>
    class array_view
    {

        public:
            static constexpr size_t npos = (size_t)-1;

            inline constexpr array_view() : buffer(NULL), count(0) { }
            inline constexpr array_view(const type_t *data, size_t length) : buffer(data), count(length) { }

            // View over a raw C array, deducing the length from the bound.
            template <size_t N>
            inline constexpr array_view(const type_t (&data)[N]) : buffer(data), count(N) { }

            // Implicit view over any contiguous container exposing begin() (convertible to
            // const type_t*) and size(). Covers array, dynamic_array, and static_array.
            template <typename container_t,
                      typename = decltype(static_cast<const type_t*>(std::declval<const container_t&>().begin())),
                      typename = decltype(std::declval<const container_t&>().size())>
            inline array_view(const container_t &container)
                : buffer(container.begin()), count(container.size()) { }

            inline constexpr const type_t& get(size_t index) const        { SIMPLEX_ASSERT(index < count); return buffer[index]; }
            inline constexpr const type_t& operator[](size_t index) const { return buffer[index];                                }
            inline constexpr const type_t& front() const                 { SIMPLEX_ASSERT(count > 0); return buffer[0];         }
            inline constexpr const type_t& back() const                  { SIMPLEX_ASSERT(count > 0); return buffer[count - 1]; }
            inline constexpr const type_t* data() const                  { return buffer;                                       }
            inline constexpr const type_t* begin() const                 { return buffer;                                       }
            inline constexpr const type_t* end() const                   { return buffer + count;                               }
            inline constexpr size_t size() const                         { return this->count;                                  }
            inline constexpr bool empty() const                          { return this->count == 0;                             }

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

            // The [pos, pos + n) slice, clamped to the end of the view. Mirrors string_view::substr.
            inline constexpr array_view<type_t>
            subview(size_t pos, size_t n = npos) const
            {
                SIMPLEX_ASSERT(pos <= this->count);
                size_t remaining = this->count - pos;
                size_t length = (n < remaining) ? n : remaining;
                return array_view<type_t>(this->buffer + pos, length);
            }

            // The leading / trailing n elements (clamped), matching std::span::first / ::last.
            inline constexpr array_view<type_t>
            first(size_t n) const
            {
                size_t length = (n < this->count) ? n : this->count;
                return array_view<type_t>(this->buffer, length);
            }

            inline constexpr array_view<type_t>
            last(size_t n) const
            {
                size_t length = (n < this->count) ? n : this->count;
                return array_view<type_t>(this->buffer + (this->count - length), length);
            }

            inline constexpr bool
            starts_with(const array_view<type_t> &prefix) const
            {
                if (prefix.count > this->count) return false;
                for (size_t i = 0; i < prefix.count; ++i)
                    if (!(this->buffer[i] == prefix.buffer[i])) return false;
                return true;
            }

            inline constexpr bool
            ends_with(const array_view<type_t> &suffix) const
            {
                if (suffix.count > this->count) return false;
                const size_t offset = this->count - suffix.count;
                for (size_t i = 0; i < suffix.count; ++i)
                    if (!(this->buffer[offset + i] == suffix.buffer[i])) return false;
                return true;
            }

            inline constexpr bool starts_with(const type_t &value) const { return this->count > 0 && this->buffer[0] == value;             }
            inline constexpr bool ends_with(const type_t &value) const   { return this->count > 0 && this->buffer[this->count - 1] == value; }

            inline constexpr size_t
            find(const type_t &value, size_t pos = 0) const
            {
                for (size_t i = pos; i < this->count; ++i)
                    if (this->buffer[i] == value) return i;
                return npos;
            }

            inline constexpr size_t
            find(const array_view<type_t> &needle, size_t pos = 0) const
            {
                if (needle.count == 0) return (pos <= this->count) ? pos : npos;
                if (needle.count > this->count) return npos;

                const size_t last = this->count - needle.count;
                for (size_t i = pos; i <= last; ++i)
                {
                    size_t j = 0;
                    for (; j < needle.count; ++j)
                        if (!(this->buffer[i + j] == needle.buffer[j])) break;
                    if (j == needle.count) return i;
                }
                return npos;
            }

            inline constexpr bool
            operator==(const array_view<type_t> &other) const
            {
                if (this->count != other.count) return false;
                for (size_t i = 0; i < this->count; ++i)
                    if (!(this->buffer[i] == other.buffer[i])) return false;
                return true;
            }

            inline constexpr bool operator!=(const array_view<type_t> &other) const { return !(*this == other);          }

        private:
            const type_t *buffer;
            size_t count;

    };

}
