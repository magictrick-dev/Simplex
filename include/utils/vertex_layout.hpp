#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include "utils/linear/vec2.hpp"
#include "utils/linear/vec3.hpp"
#include "utils/linear/vec4.hpp"

enum class Attrib : uint8_t
{
    Float1,
    Float2,
    Float3,
    Float4,
};

constexpr size_t attrib_size(Attrib a) noexcept
{
    switch (a)
    {
        case Attrib::Float1: return sizeof(float);
        case Attrib::Float2: return sizeof(float) * 2;
        case Attrib::Float3: return sizeof(float) * 3;
        case Attrib::Float4: return sizeof(float) * 4;
    }
    return 0;
}

template<Attrib A> struct AttribType;
template<> struct AttribType<Attrib::Float1> { using type = float; };
template<> struct AttribType<Attrib::Float2> { using type = vec2<float>; };
template<> struct AttribType<Attrib::Float3> { using type = vec3<float>; };
template<> struct AttribType<Attrib::Float4> { using type = vec4<float>; };

// ---

// Base layout: Stride is the total byte width of one vertex (>= sum of attributes).
// Use VertexLayout<> or AlignedVertexLayout<N> rather than instantiating this directly.
template<size_t Stride, Attrib... As>
struct VertexLayoutBase
{
    static constexpr size_t count          = sizeof...(As);
    static constexpr size_t natural_stride = (attrib_size(As) + ... + 0);
    static constexpr size_t stride         = Stride;
    static constexpr size_t padding        = Stride - natural_stride;

    static_assert(Stride >= natural_stride,
        "AlignedVertexLayout<N>: N must be >= the sum of all attribute sizes.");

    static constexpr std::array<Attrib, count> attributes = { As... };

    static constexpr std::array<size_t, count> offsets = []() consteval
    {
        std::array<size_t, count> offs{};
        size_t off = 0, i = 0;
        ((offs[i++] = off, off += attrib_size(As)), ...);
        return offs;
    }();

    std::array<std::byte, Stride> data{};

    template<size_t I> requires (I < count)
    auto& get() noexcept
    {
        using T = typename AttribType<attributes[I]>::type;
        return *reinterpret_cast<T*>(data.data() + offsets[I]);
    }

    template<size_t I> requires (I < count)
    const auto& get() const noexcept
    {
        using T = typename AttribType<attributes[I]>::type;
        return *reinterpret_cast<const T*>(data.data() + offsets[I]);
    }
};

// Natural stride: tightly packed, no padding.
template<Attrib... As>
using VertexLayout = VertexLayoutBase<(attrib_size(As) + ... + 0), As...>;

// Fixed stride: pads the vertex out to exactly N bytes.
template<size_t N, Attrib... As>
using AlignedVertexLayout = VertexLayoutBase<N, As...>;
