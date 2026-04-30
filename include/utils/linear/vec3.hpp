#pragma once
#include <utils/linear/vec2.hpp>

template <Numerical Type>
union vec3
{

    Type elements[3];

    struct
    {
        Type x;
        Type y;
        Type z;
    };

    struct
    {
        Type r;
        Type g;
        Type b;
    };

    struct
    {
        Type s;
        Type t;
        Type p;
    };

    struct
    {
        Type width;
        Type height;
        Type depth;
    };

    struct
    {
        vec2<Type> xy;
        Type _z0;
    };

    struct
    {
        Type _x0;
        vec2<Type> yz;
    };

    vec3()                       : elements{} {}
    vec3(Type x, Type y, Type z) : elements{x, y, z} {}
    vec3(vec2<Type> xy, Type z)  : elements{xy.elements[0], xy.elements[1], z} {}
    vec3(Type x, vec2<Type> yz)  : elements{x, yz.elements[0], yz.elements[1]} {}

    // Scalar compound assignment
    inline vec3<Type>& operator+=(Type s) { for (auto& e : elements) e += s; return *this; }
    inline vec3<Type>& operator-=(Type s) { for (auto& e : elements) e -= s; return *this; }
    inline vec3<Type>& operator*=(Type s) { for (auto& e : elements) e *= s; return *this; }
    inline vec3<Type>& operator/=(Type s) { for (auto& e : elements) e /= s; return *this; }

    // Component-wise compound assignment
    inline vec3<Type>& operator+=(const vec3<Type>& o) { for (int i = 0; i < 3; ++i) elements[i] += o.elements[i]; return *this; }
    inline vec3<Type>& operator-=(const vec3<Type>& o) { for (int i = 0; i < 3; ++i) elements[i] -= o.elements[i]; return *this; }
    inline vec3<Type>& operator*=(const vec3<Type>& o) { for (int i = 0; i < 3; ++i) elements[i] *= o.elements[i]; return *this; }
    inline vec3<Type>& operator/=(const vec3<Type>& o) { for (int i = 0; i < 3; ++i) elements[i] /= o.elements[i]; return *this; }

};

// --- Scalar binary operators ---

template <Numerical Type>
inline vec3<Type> operator-(const vec3<Type>& v)
{
    return {-v.elements[0], -v.elements[1], -v.elements[2]};
}

template <Numerical Type>
inline vec3<Type> operator+(vec3<Type> v, Type s)
{
    return v += s;
}

template <Numerical Type>
inline vec3<Type> operator+(Type s, const vec3<Type>& v)
{
    return v + s;
}

template <Numerical Type>
inline vec3<Type> operator-(vec3<Type> v, Type s)
{
    return v -= s;
}

template <Numerical Type>
inline vec3<Type> operator-(Type s, const vec3<Type>& v)
{
    return {s - v.elements[0], s - v.elements[1], s - v.elements[2]};
}

template <Numerical Type>
inline vec3<Type> operator*(vec3<Type> v, Type s)
{
    return v *= s;
}

template <Numerical Type>
inline vec3<Type> operator*(Type s, const vec3<Type>& v)
{
    return v * s;
}

template <Numerical Type>
inline vec3<Type> operator/(vec3<Type> v, Type s)
{
    return v /= s;
}

template <Numerical Type>
inline vec3<Type> operator/(Type s, const vec3<Type>& v)
{
    return {s / v.elements[0], s / v.elements[1], s / v.elements[2]};
}

// --- Component-wise binary operators ---

template <Numerical Type>
inline vec3<Type> operator+(vec3<Type> a, const vec3<Type>& b)
{
    return a += b;
}

template <Numerical Type>
inline vec3<Type> operator-(vec3<Type> a, const vec3<Type>& b)
{
    return a -= b;
}

template <Numerical Type>
inline vec3<Type> operator*(vec3<Type> a, const vec3<Type>& b)
{
    return a *= b;
}

template <Numerical Type>
inline vec3<Type> operator/(vec3<Type> a, const vec3<Type>& b)
{
    return a /= b;
}

// --- Math operations ---

template <Numerical Type>
inline Type dot(const vec3<Type>& a, const vec3<Type>& b)
{
    return a.elements[0]*b.elements[0] + a.elements[1]*b.elements[1] + a.elements[2]*b.elements[2];
}

template <Numerical Type>
inline Type magnitude_squared(const vec3<Type>& v)
{
    return dot(v, v);
}

template <Numerical Type>
inline auto magnitude(const vec3<Type>& v)
{
    return std::sqrt(magnitude_squared(v));
}

template <Numerical Type>
inline vec3<Type> normalize(const vec3<Type>& v)
{
    return v / static_cast<Type>(magnitude(v));
}

template <Numerical Type>
inline vec3<Type> cross(const vec3<Type>& a, const vec3<Type>& b)
{
    return {
        a.elements[1]*b.elements[2] - a.elements[2]*b.elements[1],
        a.elements[2]*b.elements[0] - a.elements[0]*b.elements[2],
        a.elements[0]*b.elements[1] - a.elements[1]*b.elements[0]
    };
}

using vec3f = vec3<float>;
using vec3d = vec3<double>;
using vec3i = vec3<int>;
using vec3u = vec3<unsigned int>;

using color3f = vec3<float>;
using point3f = vec3<float>;
