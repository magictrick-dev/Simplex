#include <utils/linear.hpp>
#include <utils/test_registry.hpp>

#if defined(SIMPLEX_ENABLE_TESTS) && SIMPLEX_ENABLE_TESTS == 1

#include <GLM/glm.hpp>
#include <GLM/ext/matrix_transform.hpp>
#include <GLM/ext/matrix_clip_space.hpp>
#include <cmath>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static constexpr float kEps = 1e-5f;

static inline bool
approx_eq(float a, float b, float eps = kEps)
{
    return std::abs(a - b) <= eps;
}

static inline glm::vec4
to_glm(vec4f v)
{
    return {v.x, v.y, v.z, v.w};
}

static inline bool
vec_eq(vec4f a, glm::vec4 b, float eps = kEps)
{
    return approx_eq(a.x, b.x, eps)
        && approx_eq(a.y, b.y, eps)
        && approx_eq(a.z, b.z, eps)
        && approx_eq(a.w, b.w, eps);
}

struct LinearTestCase
{
    bool (*fn)();
};

static bool
run_linear_test(const LinearTestCase &t)
{
    return t.fn();
}

// ---------------------------------------------------------------------------
// vec4 tests
// ---------------------------------------------------------------------------

static bool
vec4_zero_init()
{
    vec4f v{};
    return approx_eq(v.x, 0.0f) && approx_eq(v.y, 0.0f)
        && approx_eq(v.z, 0.0f) && approx_eq(v.w, 0.0f);
}

static bool
vec4_ctor_vec2_z_w()
{
    vec4f v{vec2f{1.0f, 2.0f}, 3.0f, 4.0f};
    return approx_eq(v.x, 1.0f) && approx_eq(v.y, 2.0f)
        && approx_eq(v.z, 3.0f) && approx_eq(v.w, 4.0f);
}

static bool
vec4_ctor_x_vec2_w()
{
    vec4f v{1.0f, vec2f{1.0f, 2.0f}, 4.0f};
    return approx_eq(v.x, 1.0f) && approx_eq(v.y, 1.0f)
        && approx_eq(v.z, 2.0f) && approx_eq(v.w, 4.0f);
}

static bool
vec4_ctor_x_y_vec2()
{
    vec4f v{1.0f, 2.0f, vec2f{3.0f, 4.0f}};
    return approx_eq(v.x, 1.0f) && approx_eq(v.y, 2.0f)
        && approx_eq(v.z, 3.0f) && approx_eq(v.w, 4.0f);
}

static bool
vec4_ctor_vec2_vec2()
{
    vec4f v{vec2f{1.0f, 2.0f}, vec2f{3.0f, 4.0f}};
    return approx_eq(v.x, 1.0f) && approx_eq(v.y, 2.0f)
        && approx_eq(v.z, 3.0f) && approx_eq(v.w, 4.0f);
}

static bool
vec4_ctor_vec3_w()
{
    vec4f v{vec3f{1.0f, 2.0f, 3.0f}, 4.0f};
    return approx_eq(v.x, 1.0f) && approx_eq(v.y, 2.0f)
        && approx_eq(v.z, 3.0f) && approx_eq(v.w, 4.0f);
}

static bool
vec4_ctor_x_vec3()
{
    vec4f v{1.0f, vec3f{1.0f, 2.0f, 3.0f}};
    return approx_eq(v.x, 1.0f) && approx_eq(v.y, 1.0f)
        && approx_eq(v.z, 2.0f) && approx_eq(v.w, 3.0f);
}

static bool
vec4_rgba_aliases()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    return approx_eq(a.r, a.x) && approx_eq(a.g, a.y)
        && approx_eq(a.b, a.z) && approx_eq(a.a, a.w);
}

static bool
vec4_subvector_aliases()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    return approx_eq(a.xy.x, a.x)  && approx_eq(a.xy.y, a.y)
        && approx_eq(a.zw.x, a.z)  && approx_eq(a.zw.y, a.w)
        && approx_eq(a.xyz.x, a.x) && approx_eq(a.xyz.z, a.z)
        && approx_eq(a.yz.x, a.y)  && approx_eq(a.yz.y, a.z)
        && approx_eq(a.yzw.x, a.y) && approx_eq(a.yzw.z, a.w);
}

static bool
vec4_add_scalar()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    return vec_eq(a + 2.0f, to_glm(a) + 2.0f);
}

static bool
vec4_scalar_add()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    return vec_eq(2.0f + a, 2.0f + to_glm(a));
}

static bool
vec4_sub_scalar()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    return vec_eq(a - 1.0f, to_glm(a) - 1.0f);
}

static bool
vec4_scalar_sub()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    return vec_eq(10.0f - a, 10.0f - to_glm(a));
}

static bool
vec4_mul_scalar()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    return vec_eq(a * 3.0f, to_glm(a) * 3.0f);
}

static bool
vec4_scalar_mul()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    return vec_eq(3.0f * a, 3.0f * to_glm(a));
}

static bool
vec4_div_scalar()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    return vec_eq(a / 2.0f, to_glm(a) / 2.0f);
}

static bool
vec4_scalar_div()
{
    vec4f b{0.5f, -1.0f, 2.5f, 1.0f};
    return vec_eq(12.0f / b, 12.0f / to_glm(b));
}

static bool
vec4_negate()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    return vec_eq(-a, -to_glm(a));
}

static bool
vec4_add_vec4()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    vec4f b{0.5f, -1.0f, 2.5f, 1.0f};
    return vec_eq(a + b, to_glm(a) + to_glm(b));
}

static bool
vec4_sub_vec4()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    vec4f b{0.5f, -1.0f, 2.5f, 1.0f};
    return vec_eq(a - b, to_glm(a) - to_glm(b));
}

static bool
vec4_mul_vec4()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    vec4f b{0.5f, -1.0f, 2.5f, 1.0f};
    return vec_eq(a * b, to_glm(a) * to_glm(b));
}

static bool
vec4_div_vec4()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    vec4f b{0.5f, -1.0f, 2.5f, 1.0f};
    return vec_eq(a / b, to_glm(a) / to_glm(b));
}

static bool
vec4_compound_add_vec4()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    vec4f b{0.5f, -1.0f, 2.5f, 1.0f};
    a += b;
    glm::vec4 ga{1.0f, 2.0f, 3.0f, 4.0f};
    glm::vec4 gb{0.5f, -1.0f, 2.5f, 1.0f};
    ga += gb;
    return vec_eq(a, ga);
}

static bool
vec4_compound_sub_vec4()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    vec4f b{0.5f, -1.0f, 2.5f, 1.0f};
    a -= b;
    glm::vec4 ga{1.0f, 2.0f, 3.0f, 4.0f};
    glm::vec4 gb{0.5f, -1.0f, 2.5f, 1.0f};
    ga -= gb;
    return vec_eq(a, ga);
}

static bool
vec4_compound_mul_scalar()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    a *= 2.0f;
    glm::vec4 g{1.0f, 2.0f, 3.0f, 4.0f};
    g *= 2.0f;
    return vec_eq(a, g);
}

static bool
vec4_compound_div_scalar()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    a /= 2.0f;
    glm::vec4 g{1.0f, 2.0f, 3.0f, 4.0f};
    g /= 2.0f;
    return vec_eq(a, g);
}

static bool
vec4_dot()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    vec4f b{0.5f, -1.0f, 2.5f, 1.0f};
    return approx_eq(dot(a, b), glm::dot(to_glm(a), to_glm(b)));
}

static bool
vec4_magnitude_squared()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    return approx_eq(magnitude_squared(a), glm::dot(to_glm(a), to_glm(a)));
}

static bool
vec4_magnitude()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    return approx_eq((float)magnitude(a), glm::length(to_glm(a)));
}

static bool
vec4_normalize()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    return vec_eq(normalize(a), glm::normalize(to_glm(a)));
}

static bool
vec4_homogenize_components()
{
    vec4f a{1.0f, 2.0f, 3.0f, 4.0f};
    vec4f h = homogenize(a);
    return approx_eq(h.x, a.x / a.w)
        && approx_eq(h.y, a.y / a.w)
        && approx_eq(h.z, a.z / a.w)
        && approx_eq(h.w, 1.0f);
}

static bool
vec4_homogenize_w1_fixpoint()
{
    vec4f a{2.0f, -3.0f, 4.0f, 1.0f};
    vec4f h = homogenize(a);
    return vec_eq(h, glm::vec4{a.x, a.y, a.z, 1.0f});
}

static bool
vec4_homogenize_negative_w()
{
    vec4f a{-4.0f, 8.0f, -2.0f, 2.0f};
    vec4f h = homogenize(a);
    return approx_eq(h.x, a.x / a.w)
        && approx_eq(h.y, a.y / a.w)
        && approx_eq(h.z, a.z / a.w)
        && approx_eq(h.w, 1.0f);
}

static bool
vec4_fractional_normalize()
{
    vec4f q{-0.5f, 0.6f, -0.7f, 0.8f};
    return vec_eq(normalize(q), glm::normalize(to_glm(q)));
}

SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "default zero-init",         run_linear_test, LinearTestCase, vec4_zero_init);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4(vec2, z, w)",          run_linear_test, LinearTestCase, vec4_ctor_vec2_z_w);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4(x, vec2, w)",          run_linear_test, LinearTestCase, vec4_ctor_x_vec2_w);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4(x, y, vec2)",          run_linear_test, LinearTestCase, vec4_ctor_x_y_vec2);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4(vec2, vec2)",          run_linear_test, LinearTestCase, vec4_ctor_vec2_vec2);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4(vec3, w)",             run_linear_test, LinearTestCase, vec4_ctor_vec3_w);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4(x, vec3)",             run_linear_test, LinearTestCase, vec4_ctor_x_vec3);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "rgba aliases",              run_linear_test, LinearTestCase, vec4_rgba_aliases);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "sub-vector aliases",        run_linear_test, LinearTestCase, vec4_subvector_aliases);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4 + scalar",             run_linear_test, LinearTestCase, vec4_add_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "scalar + vec4",             run_linear_test, LinearTestCase, vec4_scalar_add);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4 - scalar",             run_linear_test, LinearTestCase, vec4_sub_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "scalar - vec4",             run_linear_test, LinearTestCase, vec4_scalar_sub);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4 * scalar",             run_linear_test, LinearTestCase, vec4_mul_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "scalar * vec4",             run_linear_test, LinearTestCase, vec4_scalar_mul);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4 / scalar",             run_linear_test, LinearTestCase, vec4_div_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "scalar / vec4",             run_linear_test, LinearTestCase, vec4_scalar_div);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "unary negation",            run_linear_test, LinearTestCase, vec4_negate);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4 + vec4",               run_linear_test, LinearTestCase, vec4_add_vec4);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4 - vec4",               run_linear_test, LinearTestCase, vec4_sub_vec4);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4 * vec4",               run_linear_test, LinearTestCase, vec4_mul_vec4);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "vec4 / vec4",               run_linear_test, LinearTestCase, vec4_div_vec4);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "compound += vec4",          run_linear_test, LinearTestCase, vec4_compound_add_vec4);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "compound -= vec4",          run_linear_test, LinearTestCase, vec4_compound_sub_vec4);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "compound *= scalar",        run_linear_test, LinearTestCase, vec4_compound_mul_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "compound /= scalar",        run_linear_test, LinearTestCase, vec4_compound_div_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "dot",                       run_linear_test, LinearTestCase, vec4_dot);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "magnitude_squared",         run_linear_test, LinearTestCase, vec4_magnitude_squared);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "magnitude",                 run_linear_test, LinearTestCase, vec4_magnitude);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "normalize",                 run_linear_test, LinearTestCase, vec4_normalize);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "homogenize components",     run_linear_test, LinearTestCase, vec4_homogenize_components);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "homogenize w==1 fixpoint",  run_linear_test, LinearTestCase, vec4_homogenize_w1_fixpoint);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "homogenize negative w",     run_linear_test, LinearTestCase, vec4_homogenize_negative_w);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec4", "fractional normalize",      run_linear_test, LinearTestCase, vec4_fractional_normalize);

#endif // SIMPLEX_ENABLE_TESTS
