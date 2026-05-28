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

static inline glm::vec2
to_glm(vec2f v)
{
    return {v.x, v.y};
}

static inline bool
vec_eq(vec2f a, glm::vec2 b, float eps = kEps)
{
    return approx_eq(a.x, b.x, eps) && approx_eq(a.y, b.y, eps);
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
// vec2 tests
// ---------------------------------------------------------------------------

static bool
vec2_union_aliases()
{
    vec2f v{0.25f, 0.75f};
    return approx_eq(v.u, v.x) && approx_eq(v.v, v.y)
        && approx_eq(v.s, v.x) && approx_eq(v.t, v.y)
        && approx_eq(v.width, v.x) && approx_eq(v.height, v.y);
}

static bool
vec2_add_scalar()
{
    vec2f a{3.0f, -4.0f};
    return vec_eq(a + 2.0f, to_glm(a) + 2.0f);
}

static bool
vec2_scalar_add()
{
    vec2f a{3.0f, -4.0f};
    return vec_eq(2.0f + a, 2.0f + to_glm(a));
}

static bool
vec2_sub_scalar()
{
    vec2f a{3.0f, -4.0f};
    return vec_eq(a - 1.0f, to_glm(a) - 1.0f);
}

static bool
vec2_scalar_sub()
{
    vec2f a{3.0f, -4.0f};
    return vec_eq(10.0f - a, 10.0f - to_glm(a));
}

static bool
vec2_mul_scalar()
{
    vec2f a{3.0f, -4.0f};
    return vec_eq(a * 3.0f, to_glm(a) * 3.0f);
}

static bool
vec2_scalar_mul()
{
    vec2f a{3.0f, -4.0f};
    return vec_eq(3.0f * a, 3.0f * to_glm(a));
}

static bool
vec2_div_scalar()
{
    vec2f a{3.0f, -4.0f};
    return vec_eq(a / 2.0f, to_glm(a) / 2.0f);
}

static bool
vec2_scalar_div()
{
    vec2f b{1.5f, 2.0f};
    return vec_eq(12.0f / b, 12.0f / to_glm(b));
}

static bool
vec2_negate()
{
    vec2f a{3.0f, -4.0f};
    return vec_eq(-a, -to_glm(a));
}

static bool
vec2_add_vec2()
{
    vec2f a{3.0f, -4.0f};
    vec2f b{1.5f, 2.0f};
    return vec_eq(a + b, to_glm(a) + to_glm(b));
}

static bool
vec2_sub_vec2()
{
    vec2f a{3.0f, -4.0f};
    vec2f b{1.5f, 2.0f};
    return vec_eq(a - b, to_glm(a) - to_glm(b));
}

static bool
vec2_mul_vec2()
{
    vec2f a{3.0f, -4.0f};
    vec2f b{1.5f, 2.0f};
    return vec_eq(a * b, to_glm(a) * to_glm(b));
}

static bool
vec2_div_vec2()
{
    vec2f a{3.0f, -4.0f};
    vec2f b{1.5f, 2.0f};
    return vec_eq(a / b, to_glm(a) / to_glm(b));
}

static bool
vec2_compound_add_scalar()
{
    vec2f a{3.0f, -4.0f};
    a += 1.0f;
    glm::vec2 g{3.0f, -4.0f};
    g += 1.0f;
    return vec_eq(a, g);
}

static bool
vec2_compound_sub_scalar()
{
    vec2f a{3.0f, -4.0f};
    a -= 1.0f;
    glm::vec2 g{3.0f, -4.0f};
    g -= 1.0f;
    return vec_eq(a, g);
}

static bool
vec2_compound_mul_scalar()
{
    vec2f a{3.0f, -4.0f};
    a *= 2.0f;
    glm::vec2 g{3.0f, -4.0f};
    g *= 2.0f;
    return vec_eq(a, g);
}

static bool
vec2_compound_div_scalar()
{
    vec2f a{3.0f, -4.0f};
    a /= 2.0f;
    glm::vec2 g{3.0f, -4.0f};
    g /= 2.0f;
    return vec_eq(a, g);
}

static bool
vec2_compound_add_vec2()
{
    vec2f a{3.0f, -4.0f};
    vec2f b{1.5f, 2.0f};
    a += b;
    glm::vec2 ga{3.0f, -4.0f};
    glm::vec2 gb{1.5f, 2.0f};
    ga += gb;
    return vec_eq(a, ga);
}

static bool
vec2_compound_sub_vec2()
{
    vec2f a{3.0f, -4.0f};
    vec2f b{1.5f, 2.0f};
    a -= b;
    glm::vec2 ga{3.0f, -4.0f};
    glm::vec2 gb{1.5f, 2.0f};
    ga -= gb;
    return vec_eq(a, ga);
}

static bool
vec2_dot()
{
    vec2f a{3.0f, -4.0f};
    vec2f b{1.5f, 2.0f};
    return approx_eq(dot(a, b), glm::dot(to_glm(a), to_glm(b)));
}

static bool
vec2_magnitude_squared()
{
    vec2f a{3.0f, -4.0f};
    return approx_eq(magnitude_squared(a), glm::dot(to_glm(a), to_glm(a)));
}

static bool
vec2_magnitude()
{
    vec2f a{3.0f, -4.0f};
    return approx_eq((float)magnitude(a), glm::length(to_glm(a)));
}

static bool
vec2_normalize()
{
    vec2f a{3.0f, -4.0f};
    return vec_eq(normalize(a), glm::normalize(to_glm(a)));
}

static bool
vec2_normalize_unit_length()
{
    vec2f a{6.0f, 8.0f};
    vec2f n = normalize(a);
    return approx_eq((float)magnitude(n), 1.0f)
        && approx_eq(n.x * a.y, n.y * a.x);
}

static bool
vec2_cross_perp_dot()
{
    vec2f a{3.0f, -4.0f};
    vec2f b{1.5f, 2.0f};
    glm::vec2 ga = to_glm(a);
    glm::vec2 gb = to_glm(b);
    return approx_eq(cross(a, b), ga.x * gb.y - ga.y * gb.x);
}

static bool
vec2_fractional_mul()
{
    vec2f p{0.1f, 0.2f};
    vec2f q{0.3f, -0.4f};
    return vec_eq(p * q, to_glm(p) * to_glm(q));
}

static bool
vec2_fractional_div()
{
    vec2f p{0.1f, 0.2f};
    vec2f q{0.3f, -0.4f};
    return vec_eq(p / q, to_glm(p) / to_glm(q));
}

static bool
vec2_fractional_dot()
{
    vec2f p{0.1f, 0.2f};
    vec2f q{0.3f, -0.4f};
    return approx_eq(dot(p, q), glm::dot(to_glm(p), to_glm(q)));
}

static bool
vec2_fractional_normalize()
{
    vec2f p{0.1f, 0.2f};
    return vec_eq(normalize(p), glm::normalize(to_glm(p)));
}

static bool
vec2_large_negate()
{
    vec2f p{-100.0f, 250.5f};
    return vec_eq(-p, -to_glm(p));
}

static bool
vec2_large_scalar_div()
{
    vec2f q{0.5f, -3.0f};
    return vec_eq(1.0f / q, 1.0f / to_glm(q));
}

static bool
vec2_large_magnitude_squared()
{
    vec2f p{-100.0f, 250.5f};
    return approx_eq(magnitude_squared(p), glm::dot(to_glm(p), to_glm(p)));
}

static bool
vec2_axis_dot_orthogonal()
{
    vec2f px{1.0f, 0.0f};
    vec2f py{0.0f, 1.0f};
    return approx_eq(dot(px, py), 0.0f);
}

static bool
vec2_axis_cross_xy()
{
    vec2f px{1.0f, 0.0f};
    vec2f py{0.0f, 1.0f};
    return approx_eq(cross(px, py), 1.0f);
}

static bool
vec2_axis_cross_yx()
{
    vec2f px{1.0f, 0.0f};
    vec2f py{0.0f, 1.0f};
    return approx_eq(cross(py, px), -1.0f);
}

SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "union aliases",             run_linear_test, LinearTestCase, vec2_union_aliases);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "vec2 + scalar",             run_linear_test, LinearTestCase, vec2_add_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "scalar + vec2",             run_linear_test, LinearTestCase, vec2_scalar_add);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "vec2 - scalar",             run_linear_test, LinearTestCase, vec2_sub_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "scalar - vec2",             run_linear_test, LinearTestCase, vec2_scalar_sub);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "vec2 * scalar",             run_linear_test, LinearTestCase, vec2_mul_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "scalar * vec2",             run_linear_test, LinearTestCase, vec2_scalar_mul);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "vec2 / scalar",             run_linear_test, LinearTestCase, vec2_div_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "scalar / vec2",             run_linear_test, LinearTestCase, vec2_scalar_div);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "unary negation",            run_linear_test, LinearTestCase, vec2_negate);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "vec2 + vec2",               run_linear_test, LinearTestCase, vec2_add_vec2);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "vec2 - vec2",               run_linear_test, LinearTestCase, vec2_sub_vec2);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "vec2 * vec2",               run_linear_test, LinearTestCase, vec2_mul_vec2);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "vec2 / vec2",               run_linear_test, LinearTestCase, vec2_div_vec2);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "compound += scalar",        run_linear_test, LinearTestCase, vec2_compound_add_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "compound -= scalar",        run_linear_test, LinearTestCase, vec2_compound_sub_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "compound *= scalar",        run_linear_test, LinearTestCase, vec2_compound_mul_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "compound /= scalar",        run_linear_test, LinearTestCase, vec2_compound_div_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "compound += vec2",          run_linear_test, LinearTestCase, vec2_compound_add_vec2);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "compound -= vec2",          run_linear_test, LinearTestCase, vec2_compound_sub_vec2);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "dot",                       run_linear_test, LinearTestCase, vec2_dot);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "magnitude_squared",         run_linear_test, LinearTestCase, vec2_magnitude_squared);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "magnitude",                 run_linear_test, LinearTestCase, vec2_magnitude);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "normalize",                 run_linear_test, LinearTestCase, vec2_normalize);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "normalize unit length",     run_linear_test, LinearTestCase, vec2_normalize_unit_length);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "cross (perp-dot)",          run_linear_test, LinearTestCase, vec2_cross_perp_dot);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "fractional mul",            run_linear_test, LinearTestCase, vec2_fractional_mul);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "fractional div",            run_linear_test, LinearTestCase, vec2_fractional_div);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "fractional dot",            run_linear_test, LinearTestCase, vec2_fractional_dot);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "fractional normalize",      run_linear_test, LinearTestCase, vec2_fractional_normalize);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "large: negate",             run_linear_test, LinearTestCase, vec2_large_negate);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "large: scalar / vec2",      run_linear_test, LinearTestCase, vec2_large_scalar_div);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "large: magnitude_squared",  run_linear_test, LinearTestCase, vec2_large_magnitude_squared);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "axis: dot(x,y) == 0",       run_linear_test, LinearTestCase, vec2_axis_dot_orthogonal);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "axis: cross(x,y) == 1",     run_linear_test, LinearTestCase, vec2_axis_cross_xy);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: vec2", "axis: cross(y,x) == -1",    run_linear_test, LinearTestCase, vec2_axis_cross_yx);

#endif // SIMPLEX_ENABLE_TESTS
