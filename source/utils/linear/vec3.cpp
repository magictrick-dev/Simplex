#include <utils/linear.hpp>
#include <utils/test_registry.hpp>

#if defined(SIMPLEX_ENABLE_TESTS) && SIMPLEX_ENABLE_TESTS == 1

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <cmath>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static constexpr float kEps      = 1e-5f;
static constexpr float kEpsLoose = 1e-4f;

static inline bool
approx_eq(float a, float b, float eps = kEps)
{
    return std::abs(a - b) <= eps;
}

static inline glm::vec3
to_glm(vec3f v)
{
    return {v.x, v.y, v.z};
}

static inline bool
vec_eq(vec3f a, glm::vec3 b, float eps = kEps)
{
    return approx_eq(a.x, b.x, eps)
        && approx_eq(a.y, b.y, eps)
        && approx_eq(a.z, b.z, eps);
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
// vec3 tests
// ---------------------------------------------------------------------------

static bool
vec3_zero_init()
{
    vec3f v{};
    return approx_eq(v.x, 0.0f) && approx_eq(v.y, 0.0f) && approx_eq(v.z, 0.0f);
}

static bool
vec3_ctor_xy_z()
{
    vec3f v{vec2f{1.0f, 2.0f}, 3.0f};
    return approx_eq(v.x, 1.0f) && approx_eq(v.y, 2.0f) && approx_eq(v.z, 3.0f);
}

static bool
vec3_ctor_x_yz()
{
    vec3f v{1.0f, vec2f{2.0f, 3.0f}};
    return approx_eq(v.x, 1.0f) && approx_eq(v.y, 2.0f) && approx_eq(v.z, 3.0f);
}

static bool
vec3_rgb_aliases()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return approx_eq(a.r, a.x) && approx_eq(a.g, a.y) && approx_eq(a.b, a.z);
}

static bool
vec3_subvector_aliases()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return approx_eq(a.xy.x, a.x) && approx_eq(a.xy.y, a.y)
        && approx_eq(a.yz.x, a.y) && approx_eq(a.yz.y, a.z);
}

static bool
vec3_add_scalar()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return vec_eq(a + 2.0f, to_glm(a) + 2.0f);
}

static bool
vec3_scalar_add()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return vec_eq(2.0f + a, 2.0f + to_glm(a));
}

static bool
vec3_sub_scalar()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return vec_eq(a - 1.0f, to_glm(a) - 1.0f);
}

static bool
vec3_scalar_sub()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return vec_eq(10.0f - a, 10.0f - to_glm(a));
}

static bool
vec3_mul_scalar()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return vec_eq(a * 3.0f, to_glm(a) * 3.0f);
}

static bool
vec3_scalar_mul()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return vec_eq(3.0f * a, 3.0f * to_glm(a));
}

static bool
vec3_div_scalar()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return vec_eq(a / 2.0f, to_glm(a) / 2.0f);
}

static bool
vec3_scalar_div()
{
    vec3f b{4.0f, -1.0f, 0.5f};
    return vec_eq(12.0f / b, 12.0f / to_glm(b));
}

static bool
vec3_negate()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return vec_eq(-a, -to_glm(a));
}

static bool
vec3_add_vec3()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    vec3f b{4.0f, -1.0f, 0.5f};
    return vec_eq(a + b, to_glm(a) + to_glm(b));
}

static bool
vec3_sub_vec3()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    vec3f b{4.0f, -1.0f, 0.5f};
    return vec_eq(a - b, to_glm(a) - to_glm(b));
}

static bool
vec3_mul_vec3()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    vec3f b{4.0f, -1.0f, 0.5f};
    return vec_eq(a * b, to_glm(a) * to_glm(b));
}

static bool
vec3_div_vec3()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    vec3f b{4.0f, -1.0f, 0.5f};
    return vec_eq(a / b, to_glm(a) / to_glm(b));
}

static bool
vec3_compound_add_scalar()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    a += 1.0f;
    glm::vec3 g{1.0f, 2.0f, 3.0f};
    g += 1.0f;
    return vec_eq(a, g);
}

static bool
vec3_compound_mul_vec3()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    vec3f b{4.0f, -1.0f, 0.5f};
    a *= b;
    glm::vec3 ga{1.0f, 2.0f, 3.0f};
    glm::vec3 gb{4.0f, -1.0f, 0.5f};
    ga *= gb;
    return vec_eq(a, ga);
}

static bool
vec3_dot()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    vec3f b{4.0f, -1.0f, 0.5f};
    return approx_eq(dot(a, b), glm::dot(to_glm(a), to_glm(b)));
}

static bool
vec3_magnitude_squared()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return approx_eq(magnitude_squared(a), glm::dot(to_glm(a), to_glm(a)));
}

static bool
vec3_magnitude()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return approx_eq((float)magnitude(a), glm::length(to_glm(a)));
}

static bool
vec3_normalize()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return vec_eq(normalize(a), glm::normalize(to_glm(a)));
}

static bool
vec3_normalize_unit_length()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    return approx_eq((float)magnitude(normalize(a)), 1.0f);
}

static bool
vec3_cross()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    vec3f b{4.0f, -1.0f, 0.5f};
    return vec_eq(cross(a, b), glm::cross(to_glm(a), to_glm(b)));
}

static bool
vec3_cross_anticommutative()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    vec3f b{4.0f, -1.0f, 0.5f};
    vec3f cab = cross(a, b);
    vec3f cba = cross(b, a);
    return approx_eq(cab.x, -cba.x)
        && approx_eq(cab.y, -cba.y)
        && approx_eq(cab.z, -cba.z);
}

static bool
vec3_cross_orthogonal_to_operands()
{
    vec3f a{1.0f, 2.0f, 3.0f};
    vec3f b{4.0f, -1.0f, 0.5f};
    vec3f c = cross(a, b);
    return approx_eq(dot(c, a), 0.0f, kEpsLoose)
        && approx_eq(dot(c, b), 0.0f, kEpsLoose);
}

static bool
vec3_axis_cross_xy()
{
    vec3f c = cross(vec3f{1.0f, 0.0f, 0.0f}, vec3f{0.0f, 1.0f, 0.0f});
    return approx_eq(c.x, 0.0f) && approx_eq(c.y, 0.0f) && approx_eq(c.z, 1.0f);
}

static bool
vec3_axis_cross_yz()
{
    vec3f c = cross(vec3f{0.0f, 1.0f, 0.0f}, vec3f{0.0f, 0.0f, 1.0f});
    return approx_eq(c.x, 1.0f) && approx_eq(c.y, 0.0f) && approx_eq(c.z, 0.0f);
}

static bool
vec3_axis_cross_zx()
{
    vec3f c = cross(vec3f{0.0f, 0.0f, 1.0f}, vec3f{1.0f, 0.0f, 0.0f});
    return approx_eq(c.x, 0.0f) && approx_eq(c.y, 1.0f) && approx_eq(c.z, 0.0f);
}

static bool
vec3_negative_dominant_cross()
{
    vec3f p{-3.0f, 1.5f, -2.25f};
    vec3f q{0.5f, -4.0f, 3.0f};
    return vec_eq(cross(p, q), glm::cross(to_glm(p), to_glm(q)));
}

static bool
vec3_near_parallel_cross()
{
    vec3f p{1.0f, 0.0f, 0.0f};
    vec3f q{0.9999f, 0.01f, 0.0f};
    return vec_eq(cross(p, q), glm::cross(to_glm(p), to_glm(q)));
}

static bool
vec3_large_normalize()
{
    vec3f p{1000.0f, -500.0f, 250.0f};
    return vec_eq(normalize(p), glm::normalize(to_glm(p)));
}

static bool
vec3_large_cross()
{
    vec3f p{1000.0f, -500.0f, 250.0f};
    vec3f q{-1.0f, 2.0f, -0.5f};
    return vec_eq(cross(p, q), glm::cross(to_glm(p), to_glm(q)), kEpsLoose);
}

SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "default zero-init",              run_linear_test, LinearTestCase, vec3_zero_init);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "vec3(vec2 xy, z)",               run_linear_test, LinearTestCase, vec3_ctor_xy_z);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "vec3(x, vec2 yz)",               run_linear_test, LinearTestCase, vec3_ctor_x_yz);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "rgb aliases",                    run_linear_test, LinearTestCase, vec3_rgb_aliases);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "sub-vector xy/yz",               run_linear_test, LinearTestCase, vec3_subvector_aliases);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "vec3 + scalar",                  run_linear_test, LinearTestCase, vec3_add_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "scalar + vec3",                  run_linear_test, LinearTestCase, vec3_scalar_add);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "vec3 - scalar",                  run_linear_test, LinearTestCase, vec3_sub_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "scalar - vec3",                  run_linear_test, LinearTestCase, vec3_scalar_sub);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "vec3 * scalar",                  run_linear_test, LinearTestCase, vec3_mul_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "scalar * vec3",                  run_linear_test, LinearTestCase, vec3_scalar_mul);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "vec3 / scalar",                  run_linear_test, LinearTestCase, vec3_div_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "scalar / vec3",                  run_linear_test, LinearTestCase, vec3_scalar_div);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "unary negation",                 run_linear_test, LinearTestCase, vec3_negate);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "vec3 + vec3",                    run_linear_test, LinearTestCase, vec3_add_vec3);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "vec3 - vec3",                    run_linear_test, LinearTestCase, vec3_sub_vec3);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "vec3 * vec3",                    run_linear_test, LinearTestCase, vec3_mul_vec3);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "vec3 / vec3",                    run_linear_test, LinearTestCase, vec3_div_vec3);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "compound += scalar",             run_linear_test, LinearTestCase, vec3_compound_add_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "compound *= vec3",               run_linear_test, LinearTestCase, vec3_compound_mul_vec3);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "dot",                            run_linear_test, LinearTestCase, vec3_dot);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "magnitude_squared",              run_linear_test, LinearTestCase, vec3_magnitude_squared);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "magnitude",                      run_linear_test, LinearTestCase, vec3_magnitude);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "normalize",                      run_linear_test, LinearTestCase, vec3_normalize);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "normalize unit length",          run_linear_test, LinearTestCase, vec3_normalize_unit_length);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "cross",                          run_linear_test, LinearTestCase, vec3_cross);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "cross anti-commutative",         run_linear_test, LinearTestCase, vec3_cross_anticommutative);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "cross orthogonal to operands",   run_linear_test, LinearTestCase, vec3_cross_orthogonal_to_operands);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "axis: cross(x,y) == z",         run_linear_test, LinearTestCase, vec3_axis_cross_xy);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "axis: cross(y,z) == x",         run_linear_test, LinearTestCase, vec3_axis_cross_yz);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "axis: cross(z,x) == y",         run_linear_test, LinearTestCase, vec3_axis_cross_zx);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "negative-dominant cross",        run_linear_test, LinearTestCase, vec3_negative_dominant_cross);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "near-parallel cross",            run_linear_test, LinearTestCase, vec3_near_parallel_cross);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "large: normalize",               run_linear_test, LinearTestCase, vec3_large_normalize);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec3", "large: cross",                   run_linear_test, LinearTestCase, vec3_large_cross);

#endif // SIMPLEX_ENABLE_TESTS
