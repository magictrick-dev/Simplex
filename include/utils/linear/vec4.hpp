#pragma once
#include <utils/linear/vec3.hpp>

template <Numerical Type>
union vec4
{

    Type elements[4];

    struct
    {
        Type x;
        Type y;
        Type z;
        Type w;
    };

    struct
    {
        Type r;
        Type g;
        Type b;
        Type a;
    };

    struct
    {
        Type s;
        Type t;
        Type p;
        Type q;
    };

    struct
    {
        vec2<Type> xy;
        vec2<Type> zw;
    };

    struct
    {
        Type _x0;
        vec2<Type> yz;
        Type _w0;
    };

    struct
    {
        vec3<Type> xyz;
        Type _w1;
    };

    struct
    {
        Type _x1;
        vec3<Type> yzw;
    };

    vec4()                               : elements{} {}
    vec4(Type x, Type y, Type z, Type w) : elements{x, y, z, w} {}
    vec4(vec2<Type> xy, Type z, Type w)  : elements{xy.elements[0], xy.elements[1], z, w} {}
    vec4(Type x, vec2<Type> yz, Type w)  : elements{x, yz.elements[0], yz.elements[1], w} {}
    vec4(Type x, Type y, vec2<Type> zw)  : elements{x, y, zw.elements[0], zw.elements[1]} {}
    vec4(vec2<Type> xy, vec2<Type> zw)   : elements{xy.elements[0], xy.elements[1], zw.elements[0], zw.elements[1]} {}
    vec4(vec3<Type> xyz, Type w)         : elements{xyz.elements[0], xyz.elements[1], xyz.elements[2], w} {}
    vec4(Type x, vec3<Type> yzw)         : elements{x, yzw.elements[0], yzw.elements[1], yzw.elements[2]} {}

    // Scalar compound assignment
    inline vec4<Type>& operator+=(Type s) { for (auto& e : elements) e += s; return *this; }
    inline vec4<Type>& operator-=(Type s) { for (auto& e : elements) e -= s; return *this; }
    inline vec4<Type>& operator*=(Type s) { for (auto& e : elements) e *= s; return *this; }
    inline vec4<Type>& operator/=(Type s) { for (auto& e : elements) e /= s; return *this; }

    // Component-wise compound assignment
    inline vec4<Type>& operator+=(const vec4<Type>& o) { for (int i = 0; i < 4; ++i) elements[i] += o.elements[i]; return *this; }
    inline vec4<Type>& operator-=(const vec4<Type>& o) { for (int i = 0; i < 4; ++i) elements[i] -= o.elements[i]; return *this; }
    inline vec4<Type>& operator*=(const vec4<Type>& o) { for (int i = 0; i < 4; ++i) elements[i] *= o.elements[i]; return *this; }
    inline vec4<Type>& operator/=(const vec4<Type>& o) { for (int i = 0; i < 4; ++i) elements[i] /= o.elements[i]; return *this; }

};

template <Numerical Type>
inline vec4<Type> operator-(const vec4<Type>& v)
{
    return {
        -v.elements[0], 
        -v.elements[1], 
        -v.elements[2], 
        -v.elements[3]
    };
}

template <Numerical Type>
inline vec4<Type> operator+(vec4<Type> v, Type s)
{
    return v += s;
}

template <Numerical Type>
inline vec4<Type> operator+(Type s, const vec4<Type>& v)
{
    return v + s;
}

template <Numerical Type>
inline vec4<Type> operator-(vec4<Type> v, Type s)
{
    return v -= s;
}

template <Numerical Type>
inline vec4<Type> operator-(Type s, const vec4<Type>& v)
{
    return {
        s - v.elements[0], 
        s - v.elements[1], 
        s - v.elements[2], 
        s - v.elements[3]
    };
}

template <Numerical Type>
inline vec4<Type> operator*(vec4<Type> v, Type s)
{
    return v *= s;
}

template <Numerical Type>
inline vec4<Type> operator*(Type s, const vec4<Type>& v)
{
    return v * s;
}

template <Numerical Type>
inline vec4<Type> operator/(vec4<Type> v, Type s)
{
    return v /= s;
}

template <Numerical Type>
inline vec4<Type> operator/(Type s, const vec4<Type>& v)
{
    return {
        s / v.elements[0], 
        s / v.elements[1], 
        s / v.elements[2], 
        s / v.elements[3]
    };
}

template <Numerical Type>
inline vec4<Type> operator+(vec4<Type> a, const vec4<Type>& b)
{
    return a += b;
}

template <Numerical Type>
inline vec4<Type> operator-(vec4<Type> a, const vec4<Type>& b)
{
    return a -= b;
}

template <Numerical Type>
inline vec4<Type> operator*(vec4<Type> a, const vec4<Type>& b)
{
    return a *= b;
}

template <Numerical Type>
inline vec4<Type> operator/(vec4<Type> a, const vec4<Type>& b)
{
    return a /= b;
}

template <Numerical Type>
inline Type dot(const vec4<Type>& a, const vec4<Type>& b)
{
    return a.elements[0]*b.elements[0] + a.elements[1]*b.elements[1]
         + a.elements[2]*b.elements[2] + a.elements[3]*b.elements[3];
}

template <Numerical Type>
inline Type magnitude_squared(const vec4<Type>& v)
{
    return dot(v, v);
}

template <Numerical Type>
inline auto magnitude(const vec4<Type>& v)
{
    return std::sqrt(magnitude_squared(v));
}

template <Numerical Type>
inline vec4<Type> normalize(const vec4<Type>& v)
{
    return v / static_cast<Type>(magnitude(v));
}

// Perspective division: divides x, y, z by w and sets w = 1, mapping to Euclidean space.
template <Numerical Type>
inline vec4<Type> homogenize(const vec4<Type>& v)
{
    return {
        v.elements[0] / v.elements[3],    
        v.elements[1] / v.elements[3],
        v.elements[2] / v.elements[3],
        Type(1)
    };
}

using vec4f = vec4<float>;
using vec4d = vec4<double>;
using vec4i = vec4<int>;
using vec4u = vec4<unsigned int>;

using color4f = vec4<float>;
using point4f = vec4<float>;
