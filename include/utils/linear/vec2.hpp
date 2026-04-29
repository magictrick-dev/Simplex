#pragma once
#include <concepts>
#include <cmath>

template <typename T>
concept Numerical = std::integral<T> || std::floating_point<T>;

template <Numerical Type>
union vec2
{

    Type elements[2];

    struct
    {
        Type x;
        Type y;
    };

    struct
    {
        Type u;
        Type v;
    };

    struct
    {
        Type s;
        Type t;
    };

    struct
    {
        Type width;
        Type height;
    };

    // Scalar compound assignment
    inline vec2<Type>& operator+=(Type s) { for (auto& e : elements) e += s; return *this; }
    inline vec2<Type>& operator-=(Type s) { for (auto& e : elements) e -= s; return *this; }
    inline vec2<Type>& operator*=(Type s) { for (auto& e : elements) e *= s; return *this; }
    inline vec2<Type>& operator/=(Type s) { for (auto& e : elements) e /= s; return *this; }

    // Component-wise compound assignment
    inline vec2<Type>& operator+=(const vec2<Type>& o) { for (int i = 0; i < 2; ++i) elements[i] += o.elements[i]; return *this; }
    inline vec2<Type>& operator-=(const vec2<Type>& o) { for (int i = 0; i < 2; ++i) elements[i] -= o.elements[i]; return *this; }
    inline vec2<Type>& operator*=(const vec2<Type>& o) { for (int i = 0; i < 2; ++i) elements[i] *= o.elements[i]; return *this; }
    inline vec2<Type>& operator/=(const vec2<Type>& o) { for (int i = 0; i < 2; ++i) elements[i] /= o.elements[i]; return *this; }

};

// --- Scalar binary operators ---

template <Numerical Type>
inline vec2<Type> operator-(const vec2<Type>& v)
{
    return {-v.elements[0], -v.elements[1]};
}

template <Numerical Type>
inline vec2<Type> operator+(vec2<Type> v, Type s)
{
    return v += s;
}

template <Numerical Type>
inline vec2<Type> operator+(Type s, const vec2<Type>& v)
{
    return v + s;
}

template <Numerical Type>
inline vec2<Type> operator-(vec2<Type> v, Type s)
{
    return v -= s;
}

template <Numerical Type>
inline vec2<Type> operator-(Type s, const vec2<Type>& v)
{
    return {s - v.elements[0], s - v.elements[1]};
}

template <Numerical Type>
inline vec2<Type> operator*(vec2<Type> v, Type s)
{
    return v *= s;
}

template <Numerical Type>
inline vec2<Type> operator*(Type s, const vec2<Type>& v)
{
    return v * s;
}

template <Numerical Type>
inline vec2<Type> operator/(vec2<Type> v, Type s)
{
    return v /= s;
}

template <Numerical Type>
inline vec2<Type> operator/(Type s, const vec2<Type>& v)
{
    return {s / v.elements[0], s / v.elements[1]};
}

// --- Component-wise binary operators ---

template <Numerical Type>
inline vec2<Type> operator+(vec2<Type> a, const vec2<Type>& b)
{
    return a += b;
}

template <Numerical Type>
inline vec2<Type> operator-(vec2<Type> a, const vec2<Type>& b)
{
    return a -= b;
}

template <Numerical Type>
inline vec2<Type> operator*(vec2<Type> a, const vec2<Type>& b)
{
    return a *= b;
}

template <Numerical Type>
inline vec2<Type> operator/(vec2<Type> a, const vec2<Type>& b)
{
    return a /= b;
}

// --- Math operations ---

template <Numerical Type>
inline Type dot(const vec2<Type>& a, const vec2<Type>& b)
{
    return a.elements[0]*b.elements[0] + a.elements[1]*b.elements[1];
}

template <Numerical Type>
inline Type magnitude_squared(const vec2<Type>& v)
{
    return dot(v, v);
}

template <Numerical Type>
inline auto magnitude(const vec2<Type>& v)
{
    return std::sqrt(magnitude_squared(v));
}

template <Numerical Type>
inline vec2<Type> normalize(const vec2<Type>& v)
{
    return v / static_cast<Type>(magnitude(v));
}

// Returns the z-component of the 3D cross product (a.k.a. perp-dot); useful for winding-order tests.
template <Numerical Type>
inline Type cross(const vec2<Type>& a, const vec2<Type>& b)
{
    return a.elements[0]*b.elements[1] - a.elements[1]*b.elements[0];
}

using vec2f = vec2<float>;
using vec2d = vec2<double>;
using vec2i = vec2<int>;
using vec2u = vec2<unsigned int>;
