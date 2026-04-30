#include <utils/defs.hpp>
#include <utils/resources.hpp>
#include <cli/cli.hpp>
#include <parsers/rdview.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <GLM/glm.hpp>
#include <GLM/ext/matrix_transform.hpp>
#include <GLM/ext/matrix_clip_space.hpp>
#include <utils/linear.hpp>

#ifndef SIMPLEX_PLATFORM_INFORMATION
#   define SIMPLEX_PLATFORM_INFORMATION
#   if defined(__APPLE__)
#       define SIMPLEX_PLATFORM_TYPE "Apple MacOSX"
#       define SIMPLEX_FRONTEND_RENDERER "OpenGL"
#       define SIMPLEX_BACKEND_RENDERER "RenderView"
#   elif defined(_WIN32)
#       define SIMPLEX_PLATFORM_TYPE "Microsoft Windows Win32"
#       define SIMPLEX_FRONTEND_RENDERER "OpenGL"
#       define SIMPLEX_BACKEND_RENDERER "RenderView"
#   elif defined(__unix__)
#       define SIMPLEX_PLATFORM_TYPE "Linux UNIX"
#       define SIMPLEX_FRONTEND_RENDERER "OpenGL"
#       define SIMPLEX_BACKEND_RENDERER "RenderView"
#   else
#       define SIMPLEX_PLATFORM_TYPE "Unknown"
#       define SIMPLEX_FRONTEND_RENDERER "Unavailable"
#       define SIMPLEX_BACKEND_RENDERER "Unavailable"
#   endif
#endif

static inline void
print_engine_information()
{
    printf("Simplex Rendering Engine - Version 0.0A - 2026 Christopher DeJong - MagicTrick-Dev\n");
    printf("    - Platform              : %s\n", SIMPLEX_PLATFORM_TYPE);
    printf("    - Frontend Renderer     : %s\n", SIMPLEX_FRONTEND_RENDERER);
    printf("    - Backend Renderer      : %s\n", SIMPLEX_BACKEND_RENDERER);
}

// ---------------------------------------------------------------------------
// Test harness
// ---------------------------------------------------------------------------

struct TestResults
{
    int passed = 0;
    int failed = 0;
};

static constexpr float kEps = 1e-5f;

static inline bool
approx_eq(float a, float b, float eps = kEps)
{
    return std::abs(a - b) <= eps;
}

static inline void
check(TestResults& r, const char* name, bool ok)
{
    if (ok) { printf("  [PASS] %s\n", name);  ++r.passed; }
    else     { printf("  [FAIL] %s\n", name);  ++r.failed; }
}

// Compare our scalar to a glm scalar.
static inline void
check_scalar(TestResults& r, const char* name, float ours, float glm_val)
{
    check(r, name, approx_eq(ours, glm_val));
}

// Compare our vec2 to a glm::vec2.
static inline void
check_vec2(TestResults& r, const char* name, vec2f ours, glm::vec2 g)
{
    bool ok = approx_eq(ours.x, g.x) && approx_eq(ours.y, g.y);
    if (!ok)
        printf("    ours=(%.6f %.6f)  glm=(%.6f %.6f)\n",
               ours.x, ours.y, g.x, g.y);
    check(r, name, ok);
}

// Compare our vec3 to a glm::vec3.
static inline void
check_vec3(TestResults& r, const char* name, vec3f ours, glm::vec3 g)
{
    bool ok = approx_eq(ours.x, g.x) && approx_eq(ours.y, g.y)
           && approx_eq(ours.z, g.z);
    if (!ok)
        printf("    ours=(%.6f %.6f %.6f)  glm=(%.6f %.6f %.6f)\n",
               ours.x, ours.y, ours.z, g.x, g.y, g.z);
    check(r, name, ok);
}

// Compare our vec4 to a glm::vec4.
static inline void
check_vec4(TestResults& r, const char* name, vec4f ours, glm::vec4 g)
{
    bool ok = approx_eq(ours.x, g.x) && approx_eq(ours.y, g.y)
           && approx_eq(ours.z, g.z) && approx_eq(ours.w, g.w);
    if (!ok)
        printf("    ours=(%.6f %.6f %.6f %.6f)  glm=(%.6f %.6f %.6f %.6f)\n",
               ours.x, ours.y, ours.z, ours.w,
               g.x, g.y, g.z, g.w);
    check(r, name, ok);
}

// Compare our mat4 to a glm::mat4 (both column-major).
static inline void
check_mat4(TestResults& r, const char* name, mat4f ours, glm::mat4 g)
{
    bool ok = true;
    for (int col = 0; col < 4 && ok; ++col)
        for (int row = 0; row < 4 && ok; ++row)
            ok = approx_eq(ours.m[col][row], g[col][row]);
    if (!ok)
    {
        printf("    ours (col-major):\n");
        for (int row = 0; row < 4; ++row)
            printf("      [%.5f  %.5f  %.5f  %.5f]\n",
                   ours.m[0][row], ours.m[1][row], ours.m[2][row], ours.m[3][row]);
        printf("    glm  (col-major):\n");
        for (int row = 0; row < 4; ++row)
            printf("      [%.5f  %.5f  %.5f  %.5f]\n",
                   g[0][row], g[1][row], g[2][row], g[3][row]);
    }
    check(r, name, ok);
}

// ---------------------------------------------------------------------------
// Per-type test sections
// ---------------------------------------------------------------------------

static inline TestResults
test_vec2()
{
    TestResults r;
    printf("\n--- vec2 ---\n");

    vec2f a{3.0f, -4.0f};
    vec2f b{1.5f,  2.0f};
    glm::vec2 ga{3.0f, -4.0f};
    glm::vec2 gb{1.5f,  2.0f};

    // Construction / union aliases
    {
        vec2f uv{0.25f, 0.75f};
        check(r, "union aliases u==x and v==y", approx_eq(uv.u, uv.x) && approx_eq(uv.v, uv.y));
        check(r, "union aliases s==x and t==y", approx_eq(uv.s, uv.x) && approx_eq(uv.t, uv.y));
        check(r, "union aliases width==x and height==y", approx_eq(uv.width, uv.x) && approx_eq(uv.height, uv.y));
    }

    // Scalar arithmetic
    check_vec2(r, "vec2 + scalar",  a + 2.0f,   ga + 2.0f);
    check_vec2(r, "scalar + vec2",  2.0f + a,   2.0f + ga);
    check_vec2(r, "vec2 - scalar",  a - 1.0f,   ga - 1.0f);
    check_vec2(r, "scalar - vec2",  10.0f - a,  10.0f - ga);
    check_vec2(r, "vec2 * scalar",  a * 3.0f,   ga * 3.0f);
    check_vec2(r, "scalar * vec2",  3.0f * a,   3.0f * ga);
    check_vec2(r, "vec2 / scalar",  a / 2.0f,   ga / 2.0f);
    check_vec2(r, "scalar / vec2",  12.0f / b,  12.0f / gb);
    check_vec2(r, "unary negation", -a,         -ga);

    // Component-wise arithmetic
    check_vec2(r, "vec2 + vec2",    a + b,      ga + gb);
    check_vec2(r, "vec2 - vec2",    a - b,      ga - gb);
    check_vec2(r, "vec2 * vec2",    a * b,      ga * gb);
    check_vec2(r, "vec2 / vec2",    a / b,      ga / gb);

    // Compound assignment
    {
        vec2f t = a;  t += 1.0f;  glm::vec2 gt = ga;  gt += 1.0f;
        check_vec2(r, "vec2 += scalar", t, gt);
        t = a;  t -= 1.0f;  gt = ga;  gt -= 1.0f;
        check_vec2(r, "vec2 -= scalar", t, gt);
        t = a;  t *= 2.0f;  gt = ga;  gt *= 2.0f;
        check_vec2(r, "vec2 *= scalar", t, gt);
        t = a;  t /= 2.0f;  gt = ga;  gt /= 2.0f;
        check_vec2(r, "vec2 /= scalar", t, gt);
        t = a;  t += b;  gt = ga;  gt += gb;
        check_vec2(r, "vec2 += vec2",   t, gt);
        t = a;  t -= b;  gt = ga;  gt -= gb;
        check_vec2(r, "vec2 -= vec2",   t, gt);
    }

    // Math operations
    check_scalar(r, "dot(vec2, vec2)",       dot(a, b),               glm::dot(ga, gb));
    check_scalar(r, "magnitude_squared",     magnitude_squared(a),    glm::dot(ga, ga));
    check_scalar(r, "magnitude",             (float)magnitude(a),     glm::length(ga));
    check_vec2  (r, "normalize",             normalize(a),            glm::normalize(ga));

    // cross (perp-dot)
    {
        float ours_cross = cross(a, b);
        float glm_cross  = ga.x * gb.y - ga.y * gb.x;
        check_scalar(r, "cross (perp-dot)", ours_cross, glm_cross);
    }

    // --- variant B: small fractional values ---
    {
        vec2f p{0.1f, 0.2f};
        vec2f q{0.3f, -0.4f};
        glm::vec2 gp{0.1f, 0.2f};
        glm::vec2 gq{0.3f, -0.4f};
        check_vec2(r, "B: vec2 + scalar",   p + 0.05f,   gp + 0.05f);
        check_vec2(r, "B: scalar - vec2",   1.0f - p,    1.0f - gp);
        check_vec2(r, "B: vec2 * vec2",     p * q,       gp * gq);
        check_vec2(r, "B: vec2 / vec2",     p / q,       gp / gq);
        check_scalar(r, "B: dot",           dot(p, q),   glm::dot(gp, gq));
        check_scalar(r, "B: magnitude",     (float)magnitude(p), glm::length(gp));
        check_vec2(r,   "B: normalize",     normalize(p), glm::normalize(gp));
        check_scalar(r, "B: cross",         cross(p, q),  gp.x*gq.y - gp.y*gq.x);
    }

    // --- variant C: large and negative values ---
    {
        vec2f p{-100.0f, 250.5f};
        vec2f q{0.5f,    -3.0f};
        glm::vec2 gp{-100.0f, 250.5f};
        glm::vec2 gq{0.5f,    -3.0f};
        check_vec2(r, "C: unary negation",  -p,          -gp);
        check_vec2(r, "C: vec2 + vec2",     p + q,       gp + gq);
        check_vec2(r, "C: vec2 - vec2",     p - q,       gp - gq);
        check_vec2(r, "C: vec2 * scalar",   p * 0.01f,   gp * 0.01f);
        check_vec2(r, "C: scalar / vec2",   1.0f / q,    1.0f / gq);
        check_scalar(r, "C: dot",           dot(p, q),   glm::dot(gp, gq));
        check_scalar(r, "C: magnitude_sq",  magnitude_squared(p), glm::dot(gp, gp));
        check_vec2(r,   "C: normalize",     normalize(q), glm::normalize(gq));
    }

    // --- variant D: unit-axis vectors ---
    {
        vec2f px{1.0f, 0.0f};
        vec2f py{0.0f, 1.0f};
        glm::vec2 gpx{1.0f, 0.0f};
        glm::vec2 gpy{0.0f, 1.0f};
        check_scalar(r, "D: dot(x_axis, y_axis) == 0", dot(px, py),  glm::dot(gpx, gpy));
        check_scalar(r, "D: cross(x,y) == 1",          cross(px, py), 1.0f);
        check_scalar(r, "D: cross(y,x) == -1",         cross(py, px), -1.0f);
        check_vec2(r,   "D: normalize x_axis == x_axis", normalize(px), glm::normalize(gpx));
    }

    return r;
}

static inline TestResults
test_vec3()
{
    TestResults r;
    printf("\n--- vec3 ---\n");

    vec3f a{1.0f, 2.0f, 3.0f};
    vec3f b{4.0f, -1.0f, 0.5f};
    glm::vec3 ga{1.0f, 2.0f, 3.0f};
    glm::vec3 gb{4.0f, -1.0f, 0.5f};

    // Constructor overloads
    {
        vec2f xy{1.0f, 2.0f};
        vec2f yz{2.0f, 3.0f};
        vec3f from_xy_z{xy, 3.0f};
        vec3f from_x_yz{1.0f, yz};
        check(r, "vec3(vec2 xy, z) constructor", approx_eq(from_xy_z.x, 1.0f)
              && approx_eq(from_xy_z.y, 2.0f) && approx_eq(from_xy_z.z, 3.0f));
        check(r, "vec3(x, vec2 yz) constructor", approx_eq(from_x_yz.x, 1.0f)
              && approx_eq(from_x_yz.y, 2.0f) && approx_eq(from_x_yz.z, 3.0f));
        vec3f zero_init{};
        check(r, "vec3 default zero-init", approx_eq(zero_init.x, 0.0f)
              && approx_eq(zero_init.y, 0.0f) && approx_eq(zero_init.z, 0.0f));
    }

    // Union aliases
    {
        check(r, "vec3 union alias r==x", approx_eq(a.r, a.x));
        check(r, "vec3 union alias g==y", approx_eq(a.g, a.y));
        check(r, "vec3 union alias b==z", approx_eq(a.b, a.z));
        check(r, "vec3 sub-vector xy",    approx_eq(a.xy.x, a.x) && approx_eq(a.xy.y, a.y));
        check(r, "vec3 sub-vector yz",    approx_eq(a.yz.x, a.y) && approx_eq(a.yz.y, a.z));
    }

    // Scalar arithmetic
    check_vec3(r, "vec3 + scalar",  a + 2.0f,   ga + 2.0f);
    check_vec3(r, "scalar + vec3",  2.0f + a,   2.0f + ga);
    check_vec3(r, "vec3 - scalar",  a - 1.0f,   ga - 1.0f);
    check_vec3(r, "scalar - vec3",  10.0f - a,  10.0f - ga);
    check_vec3(r, "vec3 * scalar",  a * 3.0f,   ga * 3.0f);
    check_vec3(r, "scalar * vec3",  3.0f * a,   3.0f * ga);
    check_vec3(r, "vec3 / scalar",  a / 2.0f,   ga / 2.0f);
    check_vec3(r, "scalar / vec3",  12.0f / b,  12.0f / gb);
    check_vec3(r, "unary negation", -a,         -ga);

    // Component-wise arithmetic
    check_vec3(r, "vec3 + vec3",    a + b,      ga + gb);
    check_vec3(r, "vec3 - vec3",    a - b,      ga - gb);
    check_vec3(r, "vec3 * vec3",    a * b,      ga * gb);
    check_vec3(r, "vec3 / vec3",    a / b,      ga / gb);

    // Compound assignment
    {
        vec3f t = a;  t += 1.0f;  glm::vec3 gt = ga;  gt += 1.0f;
        check_vec3(r, "vec3 += scalar", t, gt);
        t = a;  t *= b;  gt = ga;  gt *= gb;
        check_vec3(r, "vec3 *= vec3",   t, gt);
    }

    // Math
    check_scalar(r, "dot(vec3, vec3)",   dot(a, b),             glm::dot(ga, gb));
    check_scalar(r, "magnitude_squared", magnitude_squared(a),  glm::dot(ga, ga));
    check_scalar(r, "magnitude",         (float)magnitude(a),   glm::length(ga));
    check_vec3  (r, "normalize",         normalize(a),          glm::normalize(ga));
    check_vec3  (r, "cross",             cross(a, b),           glm::cross(ga, gb));

    // Cross is anti-commutative: cross(a,b) == -cross(b,a)
    {
        vec3f cab = cross(a, b);
        vec3f cba = cross(b, a);
        check(r, "cross anti-commutative",
              approx_eq(cab.x, -cba.x) && approx_eq(cab.y, -cba.y) && approx_eq(cab.z, -cba.z));
    }

    // --- variant B: negative-dominant values ---
    {
        vec3f p{-3.0f, 1.5f, -2.25f};
        vec3f q{0.5f, -4.0f, 3.0f};
        glm::vec3 gp{-3.0f, 1.5f, -2.25f};
        glm::vec3 gq{0.5f, -4.0f, 3.0f};
        check_vec3(r, "B: vec3 + scalar",  p + 5.0f,    gp + 5.0f);
        check_vec3(r, "B: scalar - vec3",  0.0f - p,    0.0f - gp);
        check_vec3(r, "B: vec3 * vec3",    p * q,       gp * gq);
        check_vec3(r, "B: vec3 / scalar",  p / 3.0f,    gp / 3.0f);
        check_scalar(r, "B: dot",          dot(p, q),   glm::dot(gp, gq));
        check_scalar(r, "B: magnitude",    (float)magnitude(p), glm::length(gp));
        check_vec3(r,   "B: normalize",    normalize(p), glm::normalize(gp));
        check_vec3(r,   "B: cross",        cross(p, q),  glm::cross(gp, gq));
    }

    // --- variant C: nearly-parallel vectors ---
    {
        vec3f p{1.0f, 0.0f, 0.0f};
        vec3f q{0.9999f, 0.01f, 0.0f};
        glm::vec3 gp{1.0f, 0.0f, 0.0f};
        glm::vec3 gq{0.9999f, 0.01f, 0.0f};
        check_scalar(r, "C: dot near-parallel",  dot(p, q),  glm::dot(gp, gq));
        check_vec3  (r, "C: cross near-parallel", cross(p, q), glm::cross(gp, gq));
        check_vec3  (r, "C: normalize",           normalize(q), glm::normalize(gq));
    }

    // --- variant D: axis vectors and identities ---
    {
        vec3f px{1.0f, 0.0f, 0.0f};
        vec3f py{0.0f, 1.0f, 0.0f};
        vec3f pz{0.0f, 0.0f, 1.0f};
        check_scalar(r, "D: dot(x,y)==0",      dot(px, py), 0.0f);
        check_scalar(r, "D: dot(x,z)==0",      dot(px, pz), 0.0f);
        // cross(x,y) == z
        vec3f cxy = cross(px, py);
        check(r, "D: cross(x,y)==z",
              approx_eq(cxy.x,0.0f) && approx_eq(cxy.y,0.0f) && approx_eq(cxy.z,1.0f));
        // cross(y,z) == x
        vec3f cyz = cross(py, pz);
        check(r, "D: cross(y,z)==x",
              approx_eq(cyz.x,1.0f) && approx_eq(cyz.y,0.0f) && approx_eq(cyz.z,0.0f));
        // cross(z,x) == y
        vec3f czx = cross(pz, px);
        check(r, "D: cross(z,x)==y",
              approx_eq(czx.x,0.0f) && approx_eq(czx.y,1.0f) && approx_eq(czx.z,0.0f));
    }

    // --- variant E: large-magnitude values ---
    {
        vec3f p{1000.0f, -500.0f, 250.0f};
        vec3f q{-1.0f, 2.0f, -0.5f};
        glm::vec3 gp{1000.0f, -500.0f, 250.0f};
        glm::vec3 gq{-1.0f, 2.0f, -0.5f};
        check_vec3(r, "E: vec3 + vec3",       p + q,       gp + gq);
        check_vec3(r, "E: vec3 - vec3",       p - q,       gp - gq);
        check_scalar(r, "E: dot",             dot(p, q),   glm::dot(gp, gq));
        check_vec3(r,   "E: normalize large", normalize(p), glm::normalize(gp));
        check_vec3(r,   "E: cross",           cross(p, q),  glm::cross(gp, gq));
    }

    return r;
}

static inline TestResults
test_vec4()
{
    TestResults r;
    printf("\n--- vec4 ---\n");

    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    vec4f b{0.5f, -1.0f, 2.5f, 1.0f};
    glm::vec4 ga{1.0f, 2.0f, 3.0f, 4.0f};
    glm::vec4 gb{0.5f, -1.0f, 2.5f, 1.0f};

    // Constructor overloads
    {
        vec2f xy{1.0f, 2.0f};
        vec2f zw{3.0f, 4.0f};
        vec3f xyz{1.0f, 2.0f, 3.0f};

        vec4f v0{};
        check(r, "vec4 default zero-init",  approx_eq(v0.x, 0.0f) && approx_eq(v0.w, 0.0f));

        vec4f v1{xy, 3.0f, 4.0f};
        check(r, "vec4(vec2 xy, z, w)",     approx_eq(v1.x,1.0f) && approx_eq(v1.y,2.0f)
              && approx_eq(v1.z,3.0f) && approx_eq(v1.w,4.0f));

        vec4f v2{1.0f, xy, 4.0f};
        check(r, "vec4(x, vec2 yz, w)",     approx_eq(v2.x,1.0f) && approx_eq(v2.y,1.0f)
              && approx_eq(v2.z,2.0f) && approx_eq(v2.w,4.0f));

        vec4f v3{1.0f, 2.0f, zw};
        check(r, "vec4(x, y, vec2 zw)",     approx_eq(v3.x,1.0f) && approx_eq(v3.y,2.0f)
              && approx_eq(v3.z,3.0f) && approx_eq(v3.w,4.0f));

        vec4f v4{xy, zw};
        check(r, "vec4(vec2 xy, vec2 zw)",  approx_eq(v4.x,1.0f) && approx_eq(v4.y,2.0f)
              && approx_eq(v4.z,3.0f) && approx_eq(v4.w,4.0f));

        vec4f v5{xyz, 4.0f};
        check(r, "vec4(vec3 xyz, w)",       approx_eq(v5.x,1.0f) && approx_eq(v5.y,2.0f)
              && approx_eq(v5.z,3.0f) && approx_eq(v5.w,4.0f));

        vec4f v6{1.0f, xyz};
        check(r, "vec4(x, vec3 yzw)",       approx_eq(v6.x,1.0f) && approx_eq(v6.y,1.0f)
              && approx_eq(v6.z,2.0f) && approx_eq(v6.w,3.0f));
    }

    // Union aliases
    {
        check(r, "vec4 union alias r==x",  approx_eq(a.r, a.x));
        check(r, "vec4 union alias g==y",  approx_eq(a.g, a.y));
        check(r, "vec4 union alias b==z",  approx_eq(a.b, a.z));
        check(r, "vec4 union alias a==w",  approx_eq(a.a, a.w));
        check(r, "vec4 sub-vector xy",     approx_eq(a.xy.x, a.x) && approx_eq(a.xy.y, a.y));
        check(r, "vec4 sub-vector zw",     approx_eq(a.zw.x, a.z) && approx_eq(a.zw.y, a.w));
        check(r, "vec4 sub-vector xyz",    approx_eq(a.xyz.x, a.x) && approx_eq(a.xyz.y, a.y) && approx_eq(a.xyz.z, a.z));
        check(r, "vec4 sub-vector yz",     approx_eq(a.yz.x, a.y) && approx_eq(a.yz.y, a.z));
        check(r, "vec4 sub-vector yzw",    approx_eq(a.yzw.x, a.y) && approx_eq(a.yzw.y, a.z) && approx_eq(a.yzw.z, a.w));
    }

    // Scalar arithmetic
    check_vec4(r, "vec4 + scalar",  a + 2.0f,   ga + 2.0f);
    check_vec4(r, "scalar + vec4",  2.0f + a,   2.0f + ga);
    check_vec4(r, "vec4 - scalar",  a - 1.0f,   ga - 1.0f);
    check_vec4(r, "scalar - vec4",  10.0f - a,  10.0f - ga);
    check_vec4(r, "vec4 * scalar",  a * 3.0f,   ga * 3.0f);
    check_vec4(r, "scalar * vec4",  3.0f * a,   3.0f * ga);
    check_vec4(r, "vec4 / scalar",  a / 2.0f,   ga / 2.0f);
    check_vec4(r, "scalar / vec4",  12.0f / b,  12.0f / gb);
    check_vec4(r, "unary negation", -a,         -ga);

    // Component-wise arithmetic
    check_vec4(r, "vec4 + vec4",    a + b,      ga + gb);
    check_vec4(r, "vec4 - vec4",    a - b,      ga - gb);
    check_vec4(r, "vec4 * vec4",    a * b,      ga * gb);
    check_vec4(r, "vec4 / vec4",    a / b,      ga / gb);

    // Compound assignment
    {
        vec4f t = a;  t += b;  glm::vec4 gt = ga;  gt += gb;
        check_vec4(r, "vec4 += vec4", t, gt);
        t = a;  t -= b;  gt = ga;  gt -= gb;
        check_vec4(r, "vec4 -= vec4", t, gt);
        t = a;  t *= 2.0f;  gt = ga;  gt *= 2.0f;
        check_vec4(r, "vec4 *= scalar", t, gt);
        t = a;  t /= 2.0f;  gt = ga;  gt /= 2.0f;
        check_vec4(r, "vec4 /= scalar", t, gt);
    }

    // Math
    check_scalar(r, "dot(vec4, vec4)",   dot(a, b),             glm::dot(ga, gb));
    check_scalar(r, "magnitude_squared", magnitude_squared(a),  glm::dot(ga, ga));
    check_scalar(r, "magnitude",         (float)magnitude(a),   glm::length(ga));
    check_vec4  (r, "normalize",         normalize(a),          glm::normalize(ga));

    // homogenize: {x/w, y/w, z/w, 1}
    {
        vec4f h = homogenize(a);
        check(r, "homogenize x/w",  approx_eq(h.x, a.x / a.w));
        check(r, "homogenize y/w",  approx_eq(h.y, a.y / a.w));
        check(r, "homogenize z/w",  approx_eq(h.z, a.z / a.w));
        check(r, "homogenize w==1", approx_eq(h.w, 1.0f));
    }

    // --- variant B: negative and mixed-sign components ---
    {
        vec4f p{-2.0f, 5.0f, -1.5f, 3.0f};
        vec4f q{1.0f, -0.5f, 4.0f, 2.0f};
        glm::vec4 gp{-2.0f, 5.0f, -1.5f, 3.0f};
        glm::vec4 gq{1.0f, -0.5f, 4.0f, 2.0f};
        check_vec4(r, "B: vec4 + scalar",  p + 3.0f,    gp + 3.0f);
        check_vec4(r, "B: scalar - vec4",  0.0f - p,    0.0f - gp);
        check_vec4(r, "B: vec4 * vec4",    p * q,       gp * gq);
        check_vec4(r, "B: vec4 / scalar",  p / 4.0f,    gp / 4.0f);
        check_vec4(r, "B: vec4 - vec4",    p - q,       gp - gq);
        check_scalar(r, "B: dot",          dot(p, q),   glm::dot(gp, gq));
        check_scalar(r, "B: magnitude",    (float)magnitude(p), glm::length(gp));
        check_vec4(r,   "B: normalize",    normalize(p), glm::normalize(gp));
    }

    // --- variant C: homogeneous point with w != 1 ---
    {
        vec4f pts[] = {
            {6.0f,  3.0f, 9.0f,  3.0f},
            {-4.0f, 8.0f, -2.0f, 2.0f},
            {5.0f,  0.0f, 10.0f, 5.0f},
        };
        for (int i = 0; i < 3; ++i)
        {
            vec4f h = homogenize(pts[i]);
            char name[64];
            snprintf(name, sizeof(name), "C: homogenize variant %d x/w", i);
            check(r, name, approx_eq(h.x, pts[i].x / pts[i].w));
            snprintf(name, sizeof(name), "C: homogenize variant %d y/w", i);
            check(r, name, approx_eq(h.y, pts[i].y / pts[i].w));
            snprintf(name, sizeof(name), "C: homogenize variant %d z/w", i);
            check(r, name, approx_eq(h.z, pts[i].z / pts[i].w));
            snprintf(name, sizeof(name), "C: homogenize variant %d w==1", i);
            check(r, name, approx_eq(h.w, 1.0f));
        }
    }

    // --- variant D: fractional / small-magnitude values ---
    {
        vec4f p{0.1f, -0.2f, 0.3f, 0.4f};
        vec4f q{-0.5f, 0.6f, -0.7f, 0.8f};
        glm::vec4 gp{0.1f, -0.2f, 0.3f, 0.4f};
        glm::vec4 gq{-0.5f, 0.6f, -0.7f, 0.8f};
        check_vec4(r, "D: vec4 + vec4",     p + q,      gp + gq);
        check_vec4(r, "D: vec4 * scalar",   p * 10.0f,  gp * 10.0f);
        check_vec4(r, "D: scalar / vec4",   1.0f / q,   1.0f / gq);
        check_vec4(r, "D: unary negation",  -q,         -gq);
        check_scalar(r, "D: dot",           dot(p, q),  glm::dot(gp, gq));
        check_scalar(r, "D: magnitude_sq",  magnitude_squared(q), glm::dot(gq, gq));
        check_vec4  (r, "D: normalize",     normalize(q), glm::normalize(gq));
    }

    return r;
}

static inline TestResults
test_mat4()
{
    TestResults r;
    printf("\n--- mat4 ---\n");

    // Default zero-init
    {
        mat4f z{};
        bool all_zero = true;
        for (int i = 0; i < 16; ++i) all_zero = all_zero && approx_eq(z.elements[i], 0.0f);
        check(r, "mat4 default zero-init", all_zero);
    }

    // Identity
    {
        mat4f  id = identity<float>();
        glm::mat4 gid = glm::mat4(1.0f);
        check_mat4(r, "identity()", id, gid);
    }

    // Diagonal constructor
    {
        mat4f  d{3.0f};
        glm::mat4 gd{3.0f};
        check_mat4(r, "diagonal constructor", d, gd);
    }

    // Column-vector constructor
    {
        vec4f c0{1,2,3,4}, c1{5,6,7,8}, c2{9,10,11,12}, c3{13,14,15,16};
        mat4f  m{c0, c1, c2, c3};
        glm::mat4 gm{glm::vec4{1,2,3,4}, glm::vec4{5,6,7,8},
                     glm::vec4{9,10,11,12}, glm::vec4{13,14,15,16}};
        check_mat4(r, "column-vec4 constructor", m, gm);
    }

    // Column/element access
    {
        mat4f id = identity<float>();
        check(r, "operator()(col,row) diagonal",
              approx_eq(id(0,0), 1.0f) && approx_eq(id(1,1), 1.0f) &&
              approx_eq(id(2,2), 1.0f) && approx_eq(id(3,3), 1.0f));
        check(r, "operator[](col) gives column vec4",
              approx_eq(id[0].x, 1.0f) && approx_eq(id[0].y, 0.0f));
    }

    // Scalar operators
    {
        mat4f  m = identity<float>() * 4.0f;
        glm::mat4 gm = glm::mat4(1.0f) * 4.0f;
        check_mat4(r, "mat4 * scalar", m, gm);
        check_mat4(r, "scalar * mat4", 4.0f * identity<float>(), gm);
    }

    // Element-wise add / sub
    {
        mat4f  a = identity<float>() * 2.0f;
        mat4f  b = identity<float>() * 3.0f;
        mat4f  sum  = a + b;
        mat4f  diff = b - a;
        glm::mat4 ga = glm::mat4(2.0f);
        glm::mat4 gb = glm::mat4(3.0f);
        check_mat4(r, "mat4 + mat4 (element-wise)", sum,  ga + gb);
        check_mat4(r, "mat4 - mat4 (element-wise)", diff, gb - ga);
    }

    // Matrix multiply
    {
        // Two non-trivial matrices
        mat4f A{
            1,2,3,4,   5,6,7,8,   9,10,11,12,   13,14,15,16
        };
        mat4f B{
            17,18,19,20,   21,22,23,24,   25,26,27,28,   29,30,31,32
        };
        glm::mat4 gA{glm::vec4{1,2,3,4},   glm::vec4{5,6,7,8},
                     glm::vec4{9,10,11,12}, glm::vec4{13,14,15,16}};
        glm::mat4 gB{glm::vec4{17,18,19,20}, glm::vec4{21,22,23,24},
                     glm::vec4{25,26,27,28},  glm::vec4{29,30,31,32}};
        check_mat4(r, "mat4 * mat4",       A * B, gA * gB);
        check_mat4(r, "mat4 * mat4 (B*A)", B * A, gB * gA);

        // *= compound
        mat4f C = A;  C *= B;
        check_mat4(r, "mat4 *= mat4", C, gA * gB);

        // Associativity check: (A*B)*id == A*B
        mat4f id = identity<float>();
        check_mat4(r, "mat4 * identity == mat4", A * id, gA);
        check_mat4(r, "identity * mat4 == mat4", id * A, gA);
    }

    // Matrix * vector
    {
        mat4f  M = identity<float>();
        vec4f  v{1.0f, 2.0f, 3.0f, 4.0f};
        glm::mat4 gM{1.0f};
        glm::vec4 gv{1.0f, 2.0f, 3.0f, 4.0f};
        check_vec4(r, "identity * vec4", M * v, gM * gv);

        // Non-trivial transform: scale then check the result
        mat4f  S = scale(vec3f{2.0f, 3.0f, 4.0f});
        glm::mat4 gS = glm::scale(glm::mat4(1.0f), glm::vec3{2.0f, 3.0f, 4.0f});
        vec4f  sv{1.0f, 1.0f, 1.0f, 1.0f};
        glm::vec4 gsv{1.0f, 1.0f, 1.0f, 1.0f};
        check_vec4(r, "scale_mat * vec4", S * sv, gS * gsv);
    }

    // Transpose
    {
        mat4f A{1,2,3,4,  5,6,7,8,  9,10,11,12,  13,14,15,16};
        glm::mat4 gA{glm::vec4{1,2,3,4}, glm::vec4{5,6,7,8},
                     glm::vec4{9,10,11,12}, glm::vec4{13,14,15,16}};
        check_mat4(r, "transpose", transpose(A), glm::transpose(gA));

        // transpose(transpose(A)) == A
        check_mat4(r, "transpose(transpose) == original", transpose(transpose(A)), gA);
    }

    // Translate
    {
        vec3f t{3.0f, -2.0f, 7.0f};
        mat4f  T = translate(t);
        glm::mat4 gT = glm::translate(glm::mat4(1.0f), glm::vec3{3.0f, -2.0f, 7.0f});
        check_mat4(r, "translate", T, gT);

        // Applying translation to a point (w=1) moves it
        vec4f  pt{1.0f, 1.0f, 1.0f, 1.0f};
        glm::vec4 gpt{1.0f, 1.0f, 1.0f, 1.0f};
        check_vec4(r, "translate * point", T * pt, gT * gpt);

        // Applying translation to a direction (w=0) leaves it unchanged
        vec4f  dir{1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec4 gdir{1.0f, 0.0f, 0.0f, 0.0f};
        check_vec4(r, "translate * direction (w=0 unchanged)", T * dir, gT * gdir);
    }

    // Scale (vec3)
    {
        vec3f s{2.0f, 3.0f, 4.0f};
        mat4f  S = scale(s);
        glm::mat4 gS = glm::scale(glm::mat4(1.0f), glm::vec3{2.0f, 3.0f, 4.0f});
        check_mat4(r, "scale(vec3)", S, gS);
    }

    // Scale (uniform)
    {
        mat4f  S = scale(5.0f);
        glm::mat4 gS = glm::scale(glm::mat4(1.0f), glm::vec3{5.0f, 5.0f, 5.0f});
        check_mat4(r, "scale(uniform)", S, gS);
    }

    // rotate_x
    {
        float angle = 0.7854f; // ~45 degrees
        mat4f  Rx = rotate_x(angle);
        glm::mat4 gRx = glm::rotate(glm::mat4(1.0f), angle, glm::vec3{1,0,0});
        check_mat4(r, "rotate_x", Rx, gRx);
    }

    // rotate_y
    {
        float angle = 1.0472f; // ~60 degrees
        mat4f  Ry = rotate_y(angle);
        glm::mat4 gRy = glm::rotate(glm::mat4(1.0f), angle, glm::vec3{0,1,0});
        check_mat4(r, "rotate_y", Ry, gRy);
    }

    // rotate_z
    {
        float angle = 0.5236f; // ~30 degrees
        mat4f  Rz = rotate_z(angle);
        glm::mat4 gRz = glm::rotate(glm::mat4(1.0f), angle, glm::vec3{0,0,1});
        check_mat4(r, "rotate_z", Rz, gRz);
    }

    // rotate (axis-angle)
    {
        float angle = 1.2217f; // ~70 degrees
        vec3f  axis{1.0f, 1.0f, 0.0f}; // non-axis-aligned, will be normalized
        mat4f  R = rotate(angle, axis);
        glm::mat4 gR = glm::rotate(glm::mat4(1.0f), angle, glm::vec3{1.0f, 1.0f, 0.0f});
        check_mat4(r, "rotate(axis-angle)", R, gR);
    }

    // Rotation preserves vector length
    {
        mat4f Rx = rotate_x(1.234f);
        vec4f v{3.0f, -1.5f, 2.0f, 0.0f};
        vec4f rv = Rx * v;
        float len_before = (float)magnitude(vec3f{v.x, v.y, v.z});
        float len_after  = (float)magnitude(vec3f{rv.x, rv.y, rv.z});
        check(r, "rotation preserves vector length", approx_eq(len_before, len_after, 1e-4f));
    }

    // Rotation: Rx * Ry != Ry * Rx (non-commutative)
    {
        float a = 0.5f;
        mat4f AB = rotate_x(a) * rotate_y(a);
        mat4f BA = rotate_y(a) * rotate_x(a);
        bool noncommut = false;
        for (int i = 0; i < 16; ++i)
            if (!approx_eq(AB.elements[i], BA.elements[i])) { noncommut = true; break; }
        check(r, "rotations are non-commutative", noncommut);
    }

    // perspective (right-handed, NDC z in [-1,1])
    {
        float fovy   = 0.7854f; // 45 deg
        float aspect = 16.0f / 9.0f;
        float znear  = 0.1f;
        float zfar   = 1000.0f;
        mat4f  P = perspective(fovy, aspect, znear, zfar);
        glm::mat4 gP = glm::perspectiveRH_NO(fovy, aspect, znear, zfar);
        check_mat4(r, "perspective RH NO", P, gP);
    }

    // orthographic (right-handed, NDC z in [-1,1])
    {
        mat4f  O = orthographic(-10.0f, 10.0f, -7.5f, 7.5f, 0.1f, 100.0f);
        glm::mat4 gO = glm::orthoRH_NO(-10.0f, 10.0f, -7.5f, 7.5f, 0.1f, 100.0f);
        check_mat4(r, "orthographic RH NO", O, gO);
    }

    // look_at (right-handed)
    {
        vec3f  eye{0.0f, 3.0f, 5.0f};
        vec3f  center{0.0f, 0.0f, 0.0f};
        vec3f  up{0.0f, 1.0f, 0.0f};
        mat4f  V = look_at(eye, center, up);
        glm::mat4 gV = glm::lookAtRH(glm::vec3{0,3,5}, glm::vec3{0,0,0}, glm::vec3{0,1,0});
        check_mat4(r, "look_at RH", V, gV);
    }

    // look_at with non-trivial up vector
    {
        vec3f  eye{1.0f, 2.0f, 3.0f};
        vec3f  center{-1.0f, 0.0f, -2.0f};
        vec3f  up{0.0f, 1.0f, 0.0f};
        mat4f  V = look_at(eye, center, up);
        glm::mat4 gV = glm::lookAtRH(glm::vec3{1,2,3}, glm::vec3{-1,0,-2}, glm::vec3{0,1,0});
        check_mat4(r, "look_at RH (non-trivial)", V, gV);
    }

    // Combined transform: P * V * M applied to a point should match GLM
    {
        mat4f  M = translate(vec3f{1.0f, 2.0f, -3.0f}) * scale(vec3f{0.5f, 0.5f, 0.5f});
        mat4f  V = look_at(vec3f{0,5,10}, vec3f{0,0,0}, vec3f{0,1,0});
        mat4f  P = perspective(0.7854f, 16.0f/9.0f, 0.1f, 100.0f);
        mat4f  PVM = P * V * M;

        glm::mat4 gM = glm::translate(glm::mat4(1.0f), glm::vec3{1,2,-3})
                     * glm::scale(glm::mat4(1.0f), glm::vec3{0.5f,0.5f,0.5f});
        glm::mat4 gV = glm::lookAtRH(glm::vec3{0,5,10}, glm::vec3{0,0,0}, glm::vec3{0,1,0});
        glm::mat4 gP = glm::perspectiveRH_NO(0.7854f, 16.0f/9.0f, 0.1f, 100.0f);
        glm::mat4 gPVM = gP * gV * gM;

        vec4f  pt{0.0f, 0.0f, 0.0f, 1.0f};
        glm::vec4 gpt{0.0f, 0.0f, 0.0f, 1.0f};
        check_mat4(r, "combined P*V*M matrix",   PVM,       gPVM);
        check_vec4(r, "combined P*V*M * point",  PVM * pt,  gPVM * gpt);
    }

    // --- additional rotate_x angles ---
    {
        float angles[] = {0.0f, 0.1745f, 1.5708f, 3.1416f, 4.7124f};
        const char* labels[] = {"rotate_x 0deg", "rotate_x 10deg", "rotate_x 90deg",
                                "rotate_x 180deg", "rotate_x 270deg"};
        for (int i = 0; i < 5; ++i)
        {
            mat4f  Rx = rotate_x(angles[i]);
            glm::mat4 gRx = glm::rotate(glm::mat4(1.0f), angles[i], glm::vec3{1,0,0});
            check_mat4(r, labels[i], Rx, gRx);
        }
    }

    // --- additional rotate_y angles ---
    {
        float angles[] = {0.2618f, 0.5236f, 1.0472f, 2.0944f, 5.4978f};
        const char* labels[] = {"rotate_y 15deg", "rotate_y 30deg", "rotate_y 60deg",
                                "rotate_y 120deg", "rotate_y 315deg"};
        for (int i = 0; i < 5; ++i)
        {
            mat4f  Ry = rotate_y(angles[i]);
            glm::mat4 gRy = glm::rotate(glm::mat4(1.0f), angles[i], glm::vec3{0,1,0});
            check_mat4(r, labels[i], Ry, gRy);
        }
    }

    // --- additional rotate_z angles ---
    {
        float angles[] = {0.3927f, 0.7854f, 1.5708f, 2.3562f, 6.2832f};
        const char* labels[] = {"rotate_z 22.5deg", "rotate_z 45deg", "rotate_z 90deg",
                                "rotate_z 135deg", "rotate_z 360deg"};
        for (int i = 0; i < 5; ++i)
        {
            mat4f  Rz = rotate_z(angles[i]);
            glm::mat4 gRz = glm::rotate(glm::mat4(1.0f), angles[i], glm::vec3{0,0,1});
            check_mat4(r, labels[i], Rz, gRz);
        }
    }

    // --- additional axis-angle rotations ---
    {
        struct { float angle; vec3f axis; const char* label; } cases[] = {
            {0.5f,   {0.0f, 0.0f, 1.0f},                       "rotate axis +Z"},
            {1.047f, {1.0f, 0.0f, 0.0f},                       "rotate axis +X"},
            {2.094f, {0.0f, 1.0f, 0.0f},                       "rotate axis +Y"},
            {0.785f, {1.0f, 1.0f, 1.0f},                       "rotate diagonal axis"},
            {1.571f, {-1.0f, 2.0f, 0.5f},                      "rotate arbitrary axis"},
            {3.142f, {0.0f, 1.0f, 0.0f},                       "rotate 180deg around Y"},
        };
        for (auto& c : cases)
        {
            mat4f  R  = rotate(c.angle, c.axis);
            glm::mat4 gR = glm::rotate(glm::mat4(1.0f), c.angle,
                                        glm::vec3{c.axis.x, c.axis.y, c.axis.z});
            check_mat4(r, c.label, R, gR);
        }
    }

    // --- additional translate variants ---
    {
        struct { vec3f t; const char* label; } cases[] = {
            {{0.0f, 0.0f, 0.0f},      "translate zero"},
            {{-5.0f, 0.0f, 0.0f},     "translate -X"},
            {{0.0f, 100.0f, 0.0f},    "translate large +Y"},
            {{1.5f, -2.5f, 3.75f},    "translate fractional"},
            {{-10.0f, -20.0f, -30.0f},"translate all-negative"},
        };
        for (auto& c : cases)
        {
            mat4f  T  = translate(c.t);
            glm::mat4 gT = glm::translate(glm::mat4(1.0f),
                                           glm::vec3{c.t.x, c.t.y, c.t.z});
            check_mat4(r, c.label, T, gT);
        }
    }

    // --- additional scale variants ---
    {
        struct { vec3f s; const char* label; } cases[] = {
            {{1.0f, 1.0f, 1.0f},      "scale identity (1,1,1)"},
            {{0.5f, 0.5f, 0.5f},      "scale shrink 0.5"},
            {{-1.0f, 1.0f, 1.0f},     "scale reflect X"},
            {{10.0f, 0.1f, 3.0f},     "scale non-uniform mixed"},
            {{0.25f, 4.0f, 0.125f},   "scale fractions"},
        };
        for (auto& c : cases)
        {
            mat4f  S  = scale(c.s);
            glm::mat4 gS = glm::scale(glm::mat4(1.0f),
                                       glm::vec3{c.s.x, c.s.y, c.s.z});
            check_mat4(r, c.label, S, gS);
        }
    }

    // --- additional perspective variants ---
    {
        struct { float fovy; float aspect; float znear; float zfar; const char* label; } cases[] = {
            {1.0472f, 1.0f,        0.01f,  10.0f,   "perspective 60deg square"},
            {1.5708f, 16.0f/9.0f, 0.05f,  500.0f,  "perspective 90deg 16:9"},
            {0.3491f, 4.0f/3.0f,  0.1f,   50.0f,   "perspective 20deg 4:3"},
            {0.6981f, 2.35f,       0.001f, 10000.0f,"perspective 40deg anamorphic"},
        };
        for (auto& c : cases)
        {
            mat4f  P  = perspective(c.fovy, c.aspect, c.znear, c.zfar);
            glm::mat4 gP = glm::perspectiveRH_NO(c.fovy, c.aspect, c.znear, c.zfar);
            check_mat4(r, c.label, P, gP);
        }
    }

    // --- additional orthographic variants ---
    {
        struct { float l,r,b,t,n,f; const char* label; } cases[] = {
            {-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f,    "ortho unit cube"},
            {0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f,  "ortho screen-space 800x600"},
            {-50.0f, 50.0f, -37.5f, 37.5f, 0.1f, 200.0f,"ortho wide field"},
            {-0.5f, 0.5f, -0.5f, 0.5f, 0.01f, 1.0f,    "ortho small near"},
        };
        for (auto& c : cases)
        {
            mat4f  O  = orthographic(c.l, c.r, c.b, c.t, c.n, c.f);
            glm::mat4 gO = glm::orthoRH_NO(c.l, c.r, c.b, c.t, c.n, c.f);
            check_mat4(r, c.label, O, gO);
        }
    }

    // --- additional look_at variants ---
    {
        struct { vec3f eye, center, up; const char* label; } cases[] = {
            {{0,0,5},   {0,0,0},   {0,1,0},  "look_at from +Z"},
            {{5,0,0},   {0,0,0},   {0,1,0},  "look_at from +X"},
            {{0,5,0},   {0,0,0},   {0,0,-1}, "look_at from +Y"},
            {{3,4,5},   {1,2,3},   {0,1,0},  "look_at offset center"},
            {{-2,3,-1}, {4,-1,2},  {0,1,0},  "look_at all-quadrant"},
            {{10,10,10},{-5,-5,-5},{0,1,0},  "look_at long diagonal"},
        };
        for (auto& c : cases)
        {
            mat4f  V  = look_at(c.eye, c.center, c.up);
            glm::mat4 gV = glm::lookAtRH(
                glm::vec3{c.eye.x,    c.eye.y,    c.eye.z},
                glm::vec3{c.center.x, c.center.y, c.center.z},
                glm::vec3{c.up.x,     c.up.y,     c.up.z});
            check_mat4(r, c.label, V, gV);
        }
    }

    // --- additional mat4 multiply: non-sequential values ---
    {
        mat4f A{
            2.0f, -1.0f, 3.0f, 0.5f,
            0.0f,  4.0f, -2.0f, 1.0f,
            -3.0f, 0.0f, 1.5f, -0.5f,
            1.0f,  2.0f, -1.0f, 3.0f
        };
        mat4f B{
            1.5f, -0.5f, 2.0f, -1.0f,
            3.0f,  1.0f, -1.5f, 0.0f,
            -2.0f, 4.0f,  0.5f, 2.0f,
            0.5f, -2.0f,  1.0f, 0.5f
        };
        glm::mat4 gA{glm::vec4{2.0f,-1.0f,3.0f,0.5f},  glm::vec4{0.0f,4.0f,-2.0f,1.0f},
                     glm::vec4{-3.0f,0.0f,1.5f,-0.5f},  glm::vec4{1.0f,2.0f,-1.0f,3.0f}};
        glm::mat4 gB{glm::vec4{1.5f,-0.5f,2.0f,-1.0f}, glm::vec4{3.0f,1.0f,-1.5f,0.0f},
                     glm::vec4{-2.0f,4.0f,0.5f,2.0f},   glm::vec4{0.5f,-2.0f,1.0f,0.5f}};
        check_mat4(r, "mat4 multiply non-sequential A*B", A * B, gA * gB);
        check_mat4(r, "mat4 multiply non-sequential B*A", B * A, gB * gA);

        // mat4 * vec4 with non-trivial matrix
        vec4f  v{1.0f, -1.0f, 2.0f, 0.5f};
        glm::vec4 gv{1.0f, -1.0f, 2.0f, 0.5f};
        check_vec4(r, "mat4 * vec4 non-trivial A", A * v, gA * gv);
        check_vec4(r, "mat4 * vec4 non-trivial B", B * v, gB * gv);
    }

    // --- translate * scale * rotate all together, multiple configurations ---
    {
        struct { vec3f t; vec3f s; float angle; vec3f axis; const char* label; } cases[] = {
            {{1,0,0},   {2,2,2},    0.5236f, {0,1,0}, "T*S*Ry 30deg"},
            {{-3,2,1},  {0.5f,1,2}, 1.0472f, {1,0,0}, "T*S*Rx 60deg"},
            {{0,-5,3},  {1,1,0.5f}, 0.7854f, {0,0,1}, "T*S*Rz 45deg"},
            {{2,2,-2},  {3,1,2},    1.5708f, {1,1,0}, "T*S*R_diag 90deg"},
        };
        for (auto& c : cases)
        {
            mat4f  M  = translate(c.t) * scale(c.s) * rotate(c.angle, c.axis);
            glm::mat4 gM = glm::translate(glm::mat4(1.0f), glm::vec3{c.t.x,c.t.y,c.t.z})
                         * glm::scale(glm::mat4(1.0f),     glm::vec3{c.s.x,c.s.y,c.s.z})
                         * glm::rotate(glm::mat4(1.0f), c.angle, glm::vec3{c.axis.x,c.axis.y,c.axis.z});
            check_mat4(r, c.label, M, gM);
        }
    }

    // --- scalar/element-wise mat4 ops with non-identity base ---
    {
        mat4f A{
            3,1,4,1,  5,9,2,6,  5,3,5,8,  9,7,9,3
        };
        glm::mat4 gA{glm::vec4{3,1,4,1}, glm::vec4{5,9,2,6},
                     glm::vec4{5,3,5,8}, glm::vec4{9,7,9,3}};

        check_mat4(r, "mat4 * scalar 2.5",    A * 2.5f,      gA * 2.5f);
        check_mat4(r, "mat4 / scalar 3.0",    A / 3.0f,      gA / 3.0f);
        check_mat4(r, "unary negation mat4",  -A,             -gA);
        check_mat4(r, "mat4 + mat4 (pi-values)", A + A,      gA + gA);
        check_mat4(r, "mat4 - mat4 == zero",  A - A,         gA - gA);
        check_mat4(r, "transpose pi-values",  transpose(A),  glm::transpose(gA));
    }

    return r;
}

static inline void
test_linear_algebra_library()
{
    printf("\n=== Linear Algebra Library vs. GLM ===\n");

    TestResults v2 = test_vec2();
    TestResults v3 = test_vec3();
    TestResults v4 = test_vec4();
    TestResults m4 = test_mat4();

    int total_passed = v2.passed + v3.passed + v4.passed + m4.passed;
    int total_failed = v2.failed + v3.failed + v4.failed + m4.failed;
    int total        = total_passed + total_failed;

    printf("\n=== Results ===\n");
    printf("  vec2 : %d/%d passed\n", v2.passed, v2.passed + v2.failed);
    printf("  vec3 : %d/%d passed\n", v3.passed, v3.passed + v3.failed);
    printf("  vec4 : %d/%d passed\n", v4.passed, v4.passed + v4.failed);
    printf("  mat4 : %d/%d passed\n", m4.passed, m4.passed + m4.failed);
    printf("  -------------------------\n");
    printf("  Total: %d/%d passed\n", total_passed, total);
    if (total_failed == 0)
        printf("  ALL TESTS PASSED\n");
    else
        printf("  *** %d TESTS FAILED ***\n", total_failed);
    printf("\n");
}

static inline bool
fetch_and_test(std::filesystem::path path)
{

    if (!std::filesystem::exists(path)) return false;
    size_t file_size = std::filesystem::file_size(path);
    std::string file_source(file_size, '\0');
    std::ifstream file_stream(path);
    if (!file_stream.is_open()) return false;
    file_stream.read(&file_source[0], file_size);
    file_stream.close();

    int32_t errors = 0;
    std::string_view file_source_view(file_source);
    RDViewTokenizer tokenizer(file_source, path);
    while (!tokenizer.current_token_is(RDViewTokenType_EOF))
    {
        RDViewToken current = tokenizer.get_current_token();
        //std::cout << current << std::endl;
        if (current.type == RDViewTokenType_Invalid) errors++;
        tokenizer.shift();
    }

    //std::cout << tokenizer.get_current_token() << std::endl; // Should EOF token.
    return errors == 0;

}

static inline std::string 
format_path_name(int number) 
{
    if (number >= 0 && number <= 9) {
        return "0" + std::to_string(number);
    }
    return std::to_string(number);
}

static int
entry(int argc, char **argv)
{

    print_engine_information();
    CLIParser cli(argc, argv);

    // Flags.
    cli.add_flag_rule('r', "Re-run the previous session.");
    cli.add_flag_rule('M', "Enable memory diagnostics.");

    // Arguments.
    cli.add_argument_rule("--no-hardware",          {},                                              "Disable hardware acceleration.");
    cli.add_argument_rule("--memory-limit",         { CLIValueType::Integer },                       "Cap the resident memory footprint.");
    cli.add_argument_rule("--image-format",         { CLIValueType::String },                        "Override the emitted image format (PPM, BMP, ...).");
    cli.add_argument_rule("--image-size",           { CLIValueType::Integer, CLIValueType::Integer },"Force the output image dimensions.");
    cli.add_argument_rule("--run-all-tests",        {},                                              "Run the full test suite.");
    cli.add_argument_rule("--run-rdview-tests",     {},                                              "Run the rdview test suite.");
    cli.add_argument_rule("--run-memory-tests",     {},                                              "Run the memory test suite.");
    cli.add_argument_rule("--force-renderer",       { CLIValueType::String },                        "Force a specific renderer backend.");
    cli.add_argument_rule("--compare-to",           { CLIValueType::String, CLIValueType::String },  "Compare against the given renderer backends.");

    // Positionals.
    cli.add_positional_rule(1, CLIValueType::Path, "Input scene description file (.rd).");

    try
    {
        cli.parse();
    }
    catch (const CLIParseException &e)
    {
        fprintf(stderr, "CLI error: %s\n", e.what());
        cli.print_help();
        return 1;
    }

    //std::filesystem::path file_path = std::filesystem::weakly_canonical(cli.get_arg(1));
    std::string base_path = "./tests/rdview/";
    for (size_t i = 0; i < 50; ++i)
    {
        std::string file_name = "s";
        file_name += format_path_name(i+1);
        file_name += ".rd";

        std::filesystem::path file_path = std::filesystem::weakly_canonical(base_path + file_name);
        bool result = fetch_and_test(file_path);
        std::cout << file_path << ": " << (result ? "Success" : "Failed") << std::endl;

    }

    test_linear_algebra_library();

    return 0;

}

#if defined(__APPLE__) && defined(__MACH__)
#   include <GLAD/glad.h>
#   include <GLFW/glfw3.h>

    int 
    main(int argc, char **argv)
    {
        return entry(argc, argv);
    }

#endif

#if defined(__unix__)

    int 
    main(int argc, char **argv)
    {
        return entry(argc, argv);
    }

#endif

#if defined(_WIN32)
#   include <windows.h>
#   include <conio.h>

    static inline void
    construct_cli_arguments(int *input_argc, char ***input_argv)
    {

        int argc = 0;
        LPWSTR *wide_argv = CommandLineToArgvW(GetCommandLineW(), &argc);

        char **argv = (char**)malloc(sizeof(const char*)*argc);
        for (int i = 0; i < argc; ++i)
        {

            int required_size = WideCharToMultiByte(CP_ACP, 0, wide_argv[i], -1, NULL, 0, NULL, NULL);
            char *buffer = (char*)malloc(required_size);
            WideCharToMultiByte(CP_ACP, 0, wide_argv[i], -1, buffer, required_size, NULL, NULL);
            argv[i] = buffer;
            
        }

        *input_argc = argc;
        *input_argv = argv;

    }

    static inline void
    deconstruct_cli_arguments(int argc, char **argv)
    {

        for (int i = 0; i < argc; ++i)
        {

            const char *string = argv[i];
            free((char*)string);

        }

        free(argv);

    }

    int WINAPI 
    wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
    {

        // Allocate the console.
        AllocConsole();
        freopen_s((FILE**)stdout,   "CONOUT$",  "w", stdout);
        freopen_s((FILE**)stderr,   "CONOUT$",  "w", stderr);
        freopen_s((FILE**)stdin,    "CONIN$",   "r", stdin);

        // Construct the command line arguments equivalent to the C-standard format.
        int argc;
        char **argv;
        construct_cli_arguments(&argc, &argv);

        int result = entry(argc, argv);

        // Release the memory.
        deconstruct_cli_arguments(argc, argv);
        
        // Hold the console before exitting.
        printf("Press any character to continue.\n");
        const char c = _getch();
        return result;

    }

#endif