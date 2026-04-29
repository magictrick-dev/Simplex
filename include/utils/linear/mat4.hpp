#pragma once
#include <utils/linear/vec4.hpp>

// Column-major storage: elements[col*4 + row], m[col][row].
// alignas(16) ensures each column sits on a 16-byte boundary when the matrix
// itself is aligned, enabling aligned SIMD loads (SSE _mm_load_ps / NEON vld1q_f32).
template <Numerical Type>
union alignas(16) mat4
{

    Type         elements[16];
    Type         m[4][4];       // m[col][row]
    vec4<Type>   columns[4];

    mat4() : elements{} {}

    // Diagonal matrix: identity when d = 1
    explicit mat4(Type d)
        : elements{d,       Type(0), Type(0), Type(0),
                   Type(0), d,       Type(0), Type(0),
                   Type(0), Type(0), d,       Type(0),
                   Type(0), Type(0), Type(0), d      } {}

    // Column-major parameter order: each group of 4 is one column, top to bottom
    mat4(Type c0r0, Type c0r1, Type c0r2, Type c0r3,
         Type c1r0, Type c1r1, Type c1r2, Type c1r3,
         Type c2r0, Type c2r1, Type c2r2, Type c2r3,
         Type c3r0, Type c3r1, Type c3r2, Type c3r3)
        : elements{c0r0, c0r1, c0r2, c0r3,
                   c1r0, c1r1, c1r2, c1r3,
                   c2r0, c2r1, c2r2, c2r3,
                   c3r0, c3r1, c3r2, c3r3} {}

    // From 4 column vectors
    mat4(const vec4<Type>& c0, const vec4<Type>& c1,
         const vec4<Type>& c2, const vec4<Type>& c3)
        : elements{c0.elements[0], c0.elements[1], c0.elements[2], c0.elements[3],
                   c1.elements[0], c1.elements[1], c1.elements[2], c1.elements[3],
                   c2.elements[0], c2.elements[1], c2.elements[2], c2.elements[3],
                   c3.elements[0], c3.elements[1], c3.elements[2], c3.elements[3]} {}

    // Element access (col, row)
    inline Type&       operator()(int col, int row)       { return m[col][row]; }
    inline Type        operator()(int col, int row) const { return m[col][row]; }

    // Column access
    inline vec4<Type>&       operator[](int col)       { return columns[col]; }
    inline const vec4<Type>& operator[](int col) const { return columns[col]; }

    // Scalar compound assignment
    inline mat4<Type>& operator*=(Type s) { for (auto& e : elements) e *= s; return *this; }
    inline mat4<Type>& operator/=(Type s) { for (auto& e : elements) e /= s; return *this; }

    // Element-wise compound assignment
    inline mat4<Type>& operator+=(const mat4<Type>& o) { for (int i = 0; i < 16; ++i) elements[i] += o.elements[i]; return *this; }
    inline mat4<Type>& operator-=(const mat4<Type>& o) { for (int i = 0; i < 16; ++i) elements[i] -= o.elements[i]; return *this; }

    // Matrix multiply: each result column is a SIMD-friendly linear combination of A's columns
    inline mat4<Type>& operator*=(const mat4<Type>& b)
    {
        mat4<Type> r;
        for (int j = 0; j < 4; ++j)
            r.columns[j] = columns[0] * b.columns[j].elements[0]
                         + columns[1] * b.columns[j].elements[1]
                         + columns[2] * b.columns[j].elements[2]
                         + columns[3] * b.columns[j].elements[3];
        return *this = r;
    }

};

template <Numerical Type>
inline mat4<Type> operator-(const mat4<Type>& a)
{
    mat4<Type> r;
    for (int i = 0; i < 16; ++i) r.elements[i] = -a.elements[i];
    return r;
}

template <Numerical Type>
inline mat4<Type> operator+(mat4<Type> a, const mat4<Type>& b) { return a += b; }

template <Numerical Type>
inline mat4<Type> operator-(mat4<Type> a, const mat4<Type>& b) { return a -= b; }

template <Numerical Type>
inline mat4<Type> operator*(mat4<Type> a, Type s) { return a *= s; }

template <Numerical Type>
inline mat4<Type> operator*(Type s, const mat4<Type>& a) { return a * s; }

template <Numerical Type>
inline mat4<Type> operator/(mat4<Type> a, Type s) { return a /= s; }

// Matrix-matrix multiply: result.columns[j] = A * B.columns[j]
// Maps directly to 4 FMA chains on SSE/NEON (one per result column).
template <Numerical Type>
inline mat4<Type> operator*(const mat4<Type>& a, const mat4<Type>& b)
{
    mat4<Type> r;
    for (int j = 0; j < 4; ++j)
        r.columns[j] = a.columns[0] * b.columns[j].elements[0]
                     + a.columns[1] * b.columns[j].elements[1]
                     + a.columns[2] * b.columns[j].elements[2]
                     + a.columns[3] * b.columns[j].elements[3];
    return r;
}

// Matrix-vector multiply: result = M * v
// Linear combination of columns, identical pattern to mat-mat (SIMD-friendly).
template <Numerical Type>
inline vec4<Type> operator*(const mat4<Type>& m, const vec4<Type>& v)
{
    return m.columns[0] * v.elements[0]
         + m.columns[1] * v.elements[1]
         + m.columns[2] * v.elements[2]
         + m.columns[3] * v.elements[3];
}

template <Numerical Type>
inline mat4<Type> transpose(const mat4<Type>& a)
{
    return {
        a.m[0][0], a.m[1][0], a.m[2][0], a.m[3][0],
        a.m[0][1], a.m[1][1], a.m[2][1], a.m[3][1],
        a.m[0][2], a.m[1][2], a.m[2][2], a.m[3][2],
        a.m[0][3], a.m[1][3], a.m[2][3], a.m[3][3]
    };
}

template <Numerical Type>
inline mat4<Type> identity() { return mat4<Type>(Type(1)); }

// --- Transform factories (floating-point types only) ---

template <std::floating_point Type>
inline mat4<Type> translate(const vec3<Type>& t)
{
    return {
        Type(1),     Type(0),     Type(0),     Type(0),
        Type(0),     Type(1),     Type(0),     Type(0),
        Type(0),     Type(0),     Type(1),     Type(0),
        t.elements[0], t.elements[1], t.elements[2], Type(1)
    };
}

template <std::floating_point Type>
inline mat4<Type> scale(const vec3<Type>& s)
{
    return {
        s.elements[0], Type(0),       Type(0),       Type(0),
        Type(0),       s.elements[1], Type(0),       Type(0),
        Type(0),       Type(0),       s.elements[2], Type(0),
        Type(0),       Type(0),       Type(0),       Type(1)
    };
}

template <std::floating_point Type>
inline mat4<Type> scale(Type s)
{
    return {
        s,       Type(0), Type(0), Type(0),
        Type(0), s,       Type(0), Type(0),
        Type(0), Type(0), s,       Type(0),
        Type(0), Type(0), Type(0), Type(1)
    };
}

template <std::floating_point Type>
inline mat4<Type> rotate_x(Type angle)
{
    Type c = std::cos(angle), s = std::sin(angle);
    return {
        Type(1), Type(0), Type(0), Type(0),
        Type(0), c,       s,       Type(0),
        Type(0), -s,      c,       Type(0),
        Type(0), Type(0), Type(0), Type(1)
    };
}

template <std::floating_point Type>
inline mat4<Type> rotate_y(Type angle)
{
    Type c = std::cos(angle), s = std::sin(angle);
    return {
        c,       Type(0), -s,      Type(0),
        Type(0), Type(1), Type(0), Type(0),
        s,       Type(0), c,       Type(0),
        Type(0), Type(0), Type(0), Type(1)
    };
}

template <std::floating_point Type>
inline mat4<Type> rotate_z(Type angle)
{
    Type c = std::cos(angle), s = std::sin(angle);
    return {
        c,       s,       Type(0), Type(0),
        -s,      c,       Type(0), Type(0),
        Type(0), Type(0), Type(1), Type(0),
        Type(0), Type(0), Type(0), Type(1)
    };
}

// Axis-angle rotation via Rodrigues' formula (axis need not be pre-normalized)
template <std::floating_point Type>
inline mat4<Type> rotate(Type angle, const vec3<Type>& axis)
{
    vec3<Type> a = normalize(axis);
    Type c = std::cos(angle), s = std::sin(angle), t = Type(1) - c;
    Type x = a.elements[0], y = a.elements[1], z = a.elements[2];
    return {
        t*x*x + c,   t*x*y + s*z, t*x*z - s*y, Type(0),
        t*x*y - s*z, t*y*y + c,   t*y*z + s*x, Type(0),
        t*x*z + s*y, t*y*z - s*x, t*z*z + c,   Type(0),
        Type(0),     Type(0),     Type(0),      Type(1)
    };
}

// Perspective projection (right-handed, OpenGL NDC z in [-1, 1])
// fovy: vertical field of view in radians, aspect: width/height
template <std::floating_point Type>
inline mat4<Type> perspective(Type fovy, Type aspect, Type near_plane, Type far_plane)
{
    Type f  = Type(1) / std::tan(fovy * Type(0.5));
    Type ri = Type(1) / (near_plane - far_plane);
    return {
        f / aspect, Type(0), Type(0),                                   Type(0),
        Type(0),    f,       Type(0),                                   Type(0),
        Type(0),    Type(0), (far_plane + near_plane) * ri,             Type(-1),
        Type(0),    Type(0), Type(2) * far_plane * near_plane * ri,     Type(0)
    };
}

// Orthographic projection (right-handed, OpenGL NDC z in [-1, 1])
template <std::floating_point Type>
inline mat4<Type> orthographic(Type left, Type right, Type bottom, Type top,
                               Type near_plane, Type far_plane)
{
    Type rl = Type(1) / (right - left);
    Type tb = Type(1) / (top - bottom);
    Type fn = Type(1) / (far_plane - near_plane);
    return {
        Type(2) * rl,            Type(0),                 Type(0),          Type(0),
        Type(0),                 Type(2) * tb,            Type(0),          Type(0),
        Type(0),                 Type(0),                 Type(-2) * fn,    Type(0),
        -(right + left) * rl,  -(top + bottom) * tb,  -(far_plane + near_plane) * fn,  Type(1)
    };
}

// View matrix (right-handed): transforms world space into camera space
template <std::floating_point Type>
inline mat4<Type> look_at(const vec3<Type>& eye, const vec3<Type>& center, const vec3<Type>& up)
{
    vec3<Type> f = normalize(center - eye);
    vec3<Type> r = normalize(cross(f, up));
    vec3<Type> u = cross(r, f);
    return {
        r.elements[0],  u.elements[0],  -f.elements[0], Type(0),
        r.elements[1],  u.elements[1],  -f.elements[1], Type(0),
        r.elements[2],  u.elements[2],  -f.elements[2], Type(0),
        -dot(r, eye),  -dot(u, eye),     dot(f, eye),   Type(1)
    };
}

using mat4f = mat4<float>;
using mat4d = mat4<double>;
using mat4i = mat4<int>;
