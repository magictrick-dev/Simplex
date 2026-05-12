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

static inline bool
mat_eq(mat4f a, glm::mat4 b, float eps = kEps)
{
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            if (!approx_eq(a.m[c][r], b[c][r], eps)) return false;
    return true;
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
// mat4 tests
// ---------------------------------------------------------------------------

static bool
mat4_zero_init()
{
    mat4f z{};
    for (int i = 0; i < 16; ++i)
        if (!approx_eq(z.elements[i], 0.0f)) return false;
    return true;
}

static bool
mat4_identity()
{
    return mat_eq(identity<float>(), glm::mat4(1.0f));
}

static bool
mat4_diagonal_ctor()
{
    return mat_eq(mat4f{3.0f}, glm::mat4{3.0f});
}

static bool
mat4_column_vec4_ctor()
{
    mat4f m{vec4f{1, 2, 3, 4}, vec4f{5, 6, 7, 8}, vec4f{9, 10, 11, 12}, vec4f{13, 14, 15, 16}};
    glm::mat4 gm{glm::vec4{1, 2, 3, 4}, glm::vec4{5, 6, 7, 8},
                 glm::vec4{9, 10, 11, 12}, glm::vec4{13, 14, 15, 16}};
    return mat_eq(m, gm);
}

static bool
mat4_element_access_diagonal()
{
    mat4f id = identity<float>();
    return approx_eq(id(0, 0), 1.0f) && approx_eq(id(1, 1), 1.0f)
        && approx_eq(id(2, 2), 1.0f) && approx_eq(id(3, 3), 1.0f)
        && approx_eq(id(0, 1), 0.0f) && approx_eq(id(2, 3), 0.0f);
}

static bool
mat4_column_access()
{
    mat4f id = identity<float>();
    return approx_eq(id[0].x, 1.0f) && approx_eq(id[0].y, 0.0f)
        && approx_eq(id[1].y, 1.0f) && approx_eq(id[3].w, 1.0f);
}

static bool
mat4_mul_scalar()
{
    return mat_eq(identity<float>() * 4.0f, glm::mat4(1.0f) * 4.0f);
}

static bool
mat4_scalar_mul()
{
    return mat_eq(4.0f * identity<float>(), 4.0f * glm::mat4(1.0f));
}

static bool
mat4_add_mat4()
{
    mat4f a = identity<float>() * 2.0f;
    mat4f b = identity<float>() * 3.0f;
    return mat_eq(a + b, glm::mat4(2.0f) + glm::mat4(3.0f));
}

static bool
mat4_sub_mat4()
{
    mat4f a = identity<float>() * 2.0f;
    mat4f b = identity<float>() * 3.0f;
    return mat_eq(b - a, glm::mat4(3.0f) - glm::mat4(2.0f));
}

static bool
mat4_mul_mat4()
{
    mat4f A{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    mat4f B{17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
    glm::mat4 gA{glm::vec4{1, 2, 3, 4},   glm::vec4{5, 6, 7, 8},
                 glm::vec4{9, 10, 11, 12}, glm::vec4{13, 14, 15, 16}};
    glm::mat4 gB{glm::vec4{17, 18, 19, 20}, glm::vec4{21, 22, 23, 24},
                 glm::vec4{25, 26, 27, 28},  glm::vec4{29, 30, 31, 32}};
    return mat_eq(A * B, gA * gB) && mat_eq(B * A, gB * gA);
}

static bool
mat4_compound_mul_mat4()
{
    mat4f A{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    mat4f B{17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
    glm::mat4 gA{glm::vec4{1, 2, 3, 4},   glm::vec4{5, 6, 7, 8},
                 glm::vec4{9, 10, 11, 12}, glm::vec4{13, 14, 15, 16}};
    glm::mat4 gB{glm::vec4{17, 18, 19, 20}, glm::vec4{21, 22, 23, 24},
                 glm::vec4{25, 26, 27, 28},  glm::vec4{29, 30, 31, 32}};
    A *= B;
    return mat_eq(A, gA * gB);
}

static bool
mat4_mul_identity_right()
{
    mat4f A{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    glm::mat4 gA{glm::vec4{1, 2, 3, 4},   glm::vec4{5, 6, 7, 8},
                 glm::vec4{9, 10, 11, 12}, glm::vec4{13, 14, 15, 16}};
    return mat_eq(A * identity<float>(), gA);
}

static bool
mat4_mul_identity_left()
{
    mat4f A{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    glm::mat4 gA{glm::vec4{1, 2, 3, 4},   glm::vec4{5, 6, 7, 8},
                 glm::vec4{9, 10, 11, 12}, glm::vec4{13, 14, 15, 16}};
    return mat_eq(identity<float>() * A, gA);
}

static bool
mat4_mul_vec4_identity()
{
    vec4f v{1.0f, 2.0f, 3.0f, 4.0f};
    return vec_eq(identity<float>() * v, to_glm(v));
}

static bool
mat4_mul_vec4_scale()
{
    mat4f S = scale(vec3f{2.0f, 3.0f, 4.0f});
    glm::mat4 gS = glm::scale(glm::mat4(1.0f), glm::vec3{2.0f, 3.0f, 4.0f});
    vec4f v{1.0f, 1.0f, 1.0f, 1.0f};
    return vec_eq(S * v, gS * glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
}

static bool
mat4_transpose()
{
    mat4f A{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    glm::mat4 gA{glm::vec4{1, 2, 3, 4},   glm::vec4{5, 6, 7, 8},
                 glm::vec4{9, 10, 11, 12}, glm::vec4{13, 14, 15, 16}};
    return mat_eq(transpose(A), glm::transpose(gA));
}

static bool
mat4_double_transpose()
{
    mat4f A{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    glm::mat4 gA{glm::vec4{1, 2, 3, 4},   glm::vec4{5, 6, 7, 8},
                 glm::vec4{9, 10, 11, 12}, glm::vec4{13, 14, 15, 16}};
    return mat_eq(transpose(transpose(A)), gA);
}

static bool
mat4_translate()
{
    return mat_eq(translate(vec3f{3.0f, -2.0f, 7.0f}),
                  glm::translate(glm::mat4(1.0f), glm::vec3{3.0f, -2.0f, 7.0f}));
}

static bool
mat4_translate_point()
{
    mat4f T = translate(vec3f{3.0f, -2.0f, 7.0f});
    glm::mat4 gT = glm::translate(glm::mat4(1.0f), glm::vec3{3.0f, -2.0f, 7.0f});
    vec4f p{1.0f, 1.0f, 1.0f, 1.0f};
    return vec_eq(T * p, gT * glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
}

static bool
mat4_translate_direction()
{
    mat4f T = translate(vec3f{3.0f, -2.0f, 7.0f});
    glm::mat4 gT = glm::translate(glm::mat4(1.0f), glm::vec3{3.0f, -2.0f, 7.0f});
    vec4f d{1.0f, 0.0f, 0.0f, 0.0f};
    return vec_eq(T * d, gT * glm::vec4{1.0f, 0.0f, 0.0f, 0.0f});
}

static bool
mat4_scale_vec3()
{
    return mat_eq(scale(vec3f{2.0f, 3.0f, 4.0f}),
                  glm::scale(glm::mat4(1.0f), glm::vec3{2.0f, 3.0f, 4.0f}));
}

static bool
mat4_scale_uniform()
{
    return mat_eq(scale(5.0f),
                  glm::scale(glm::mat4(1.0f), glm::vec3{5.0f, 5.0f, 5.0f}));
}

static bool
mat4_rotate_x()
{
    float a = 0.7854f;
    return mat_eq(rotate_x(a), glm::rotate(glm::mat4(1.0f), a, glm::vec3{1, 0, 0}));
}

static bool
mat4_rotate_y()
{
    float a = 1.0472f;
    return mat_eq(rotate_y(a), glm::rotate(glm::mat4(1.0f), a, glm::vec3{0, 1, 0}));
}

static bool
mat4_rotate_z()
{
    float a = 0.5236f;
    return mat_eq(rotate_z(a), glm::rotate(glm::mat4(1.0f), a, glm::vec3{0, 0, 1}));
}

static bool
mat4_rotate_axis_angle()
{
    float a = 1.2217f;
    return mat_eq(rotate(a, vec3f{1.0f, 1.0f, 0.0f}),
                  glm::rotate(glm::mat4(1.0f), a, glm::vec3{1.0f, 1.0f, 0.0f}));
}

static bool
mat4_rotation_preserves_length()
{
    mat4f Rx = rotate_x(1.234f);
    vec4f v{3.0f, -1.5f, 2.0f, 0.0f};
    vec4f rv = Rx * v;
    float before = (float)magnitude(vec3f{v.x, v.y, v.z});
    float after  = (float)magnitude(vec3f{rv.x, rv.y, rv.z});
    return approx_eq(before, after, kEpsLoose);
}

static bool
mat4_rotations_noncommutative()
{
    float a = 0.5f;
    mat4f AB = rotate_x(a) * rotate_y(a);
    mat4f BA = rotate_y(a) * rotate_x(a);
    for (int i = 0; i < 16; ++i)
        if (!approx_eq(AB.elements[i], BA.elements[i])) return true;
    return false;
}

static bool
mat4_rotate_x_identity()
{
    return mat_eq(rotate_x(0.0f), glm::mat4(1.0f), kEpsLoose);
}

static bool
mat4_rotate_y_identity()
{
    return mat_eq(rotate_y(0.0f), glm::mat4(1.0f), kEpsLoose);
}

static bool
mat4_rotate_z_identity()
{
    return mat_eq(rotate_z(0.0f), glm::mat4(1.0f), kEpsLoose);
}

static bool
mat4_rotate_x_90deg()
{
    return mat_eq(rotate_x(1.5708f),
                  glm::rotate(glm::mat4(1.0f), 1.5708f, glm::vec3{1, 0, 0}));
}

static bool
mat4_rotate_y_120deg()
{
    return mat_eq(rotate_y(2.0944f),
                  glm::rotate(glm::mat4(1.0f), 2.0944f, glm::vec3{0, 1, 0}));
}

static bool
mat4_rotate_z_360deg()
{
    return mat_eq(rotate_z(6.2832f),
                  glm::rotate(glm::mat4(1.0f), 6.2832f, glm::vec3{0, 0, 1}), kEpsLoose);
}

static bool
mat4_rotate_180_around_y()
{
    return mat_eq(rotate(3.142f, vec3f{0.0f, 1.0f, 0.0f}),
                  glm::rotate(glm::mat4(1.0f), 3.142f, glm::vec3{0.0f, 1.0f, 0.0f}), kEpsLoose);
}

static bool
mat4_rotate_arbitrary_axis()
{
    return mat_eq(rotate(1.571f, vec3f{-1.0f, 2.0f, 0.5f}),
                  glm::rotate(glm::mat4(1.0f), 1.571f, glm::vec3{-1.0f, 2.0f, 0.5f}));
}

static bool
mat4_perspective_45deg_16_9()
{
    return mat_eq(perspective(0.7854f, 16.0f / 9.0f, 0.1f, 1000.0f),
                  glm::perspectiveRH_NO(0.7854f, 16.0f / 9.0f, 0.1f, 1000.0f));
}

static bool
mat4_perspective_60deg_square()
{
    return mat_eq(perspective(1.0472f, 1.0f, 0.01f, 10.0f),
                  glm::perspectiveRH_NO(1.0472f, 1.0f, 0.01f, 10.0f));
}

static bool
mat4_perspective_20deg_4_3()
{
    return mat_eq(perspective(0.3491f, 4.0f / 3.0f, 0.1f, 50.0f),
                  glm::perspectiveRH_NO(0.3491f, 4.0f / 3.0f, 0.1f, 50.0f));
}

static bool
mat4_perspective_deep_frustum()
{
    return mat_eq(perspective(0.6981f, 2.35f, 0.001f, 10000.0f),
                  glm::perspectiveRH_NO(0.6981f, 2.35f, 0.001f, 10000.0f), kEpsLoose);
}

static bool
mat4_orthographic_asymmetric()
{
    return mat_eq(orthographic(-10.0f, 10.0f, -7.5f, 7.5f, 0.1f, 100.0f),
                  glm::orthoRH_NO(-10.0f, 10.0f, -7.5f, 7.5f, 0.1f, 100.0f));
}

static bool
mat4_orthographic_unit_cube()
{
    return mat_eq(orthographic(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f),
                  glm::orthoRH_NO(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f));
}

static bool
mat4_orthographic_screen_space()
{
    return mat_eq(orthographic(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f),
                  glm::orthoRH_NO(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f));
}

static bool
mat4_look_at_from_z()
{
    return mat_eq(look_at(vec3f{0.0f, 3.0f, 5.0f}, vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, 1.0f, 0.0f}),
                  glm::lookAtRH(glm::vec3{0, 3, 5}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0}));
}

static bool
mat4_look_at_nontrivial()
{
    return mat_eq(look_at(vec3f{1.0f, 2.0f, 3.0f}, vec3f{-1.0f, 0.0f, -2.0f}, vec3f{0.0f, 1.0f, 0.0f}),
                  glm::lookAtRH(glm::vec3{1, 2, 3}, glm::vec3{-1, 0, -2}, glm::vec3{0, 1, 0}));
}

static bool
mat4_look_at_from_x()
{
    return mat_eq(look_at(vec3f{5.0f, 0.0f, 0.0f}, vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, 1.0f, 0.0f}),
                  glm::lookAtRH(glm::vec3{5, 0, 0}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0}));
}

static bool
mat4_look_at_offset_center()
{
    return mat_eq(look_at(vec3f{3.0f, 4.0f, 5.0f}, vec3f{1.0f, 2.0f, 3.0f}, vec3f{0.0f, 1.0f, 0.0f}),
                  glm::lookAtRH(glm::vec3{3, 4, 5}, glm::vec3{1, 2, 3}, glm::vec3{0, 1, 0}));
}

static bool
mat4_look_at_long_diagonal()
{
    return mat_eq(look_at(vec3f{10.0f, 10.0f, 10.0f}, vec3f{-5.0f, -5.0f, -5.0f}, vec3f{0.0f, 1.0f, 0.0f}),
                  glm::lookAtRH(glm::vec3{10, 10, 10}, glm::vec3{-5, -5, -5}, glm::vec3{0, 1, 0}));
}

static bool
mat4_combined_pvm_matrix()
{
    mat4f M = translate(vec3f{1.0f, 2.0f, -3.0f}) * scale(vec3f{0.5f, 0.5f, 0.5f});
    mat4f V = look_at(vec3f{0.0f, 5.0f, 10.0f}, vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, 1.0f, 0.0f});
    mat4f P = perspective(0.7854f, 16.0f / 9.0f, 0.1f, 100.0f);
    glm::mat4 gM = glm::translate(glm::mat4(1.0f), glm::vec3{1, 2, -3})
                 * glm::scale(glm::mat4(1.0f), glm::vec3{0.5f, 0.5f, 0.5f});
    glm::mat4 gV = glm::lookAtRH(glm::vec3{0, 5, 10}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});
    glm::mat4 gP = glm::perspectiveRH_NO(0.7854f, 16.0f / 9.0f, 0.1f, 100.0f);
    return mat_eq(P * V * M, gP * gV * gM, kEpsLoose);
}

static bool
mat4_combined_pvm_point()
{
    mat4f M = translate(vec3f{1.0f, 2.0f, -3.0f}) * scale(vec3f{0.5f, 0.5f, 0.5f});
    mat4f V = look_at(vec3f{0.0f, 5.0f, 10.0f}, vec3f{0.0f, 0.0f, 0.0f}, vec3f{0.0f, 1.0f, 0.0f});
    mat4f P = perspective(0.7854f, 16.0f / 9.0f, 0.1f, 100.0f);
    glm::mat4 gM = glm::translate(glm::mat4(1.0f), glm::vec3{1, 2, -3})
                 * glm::scale(glm::mat4(1.0f), glm::vec3{0.5f, 0.5f, 0.5f});
    glm::mat4 gV = glm::lookAtRH(glm::vec3{0, 5, 10}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});
    glm::mat4 gP = glm::perspectiveRH_NO(0.7854f, 16.0f / 9.0f, 0.1f, 100.0f);
    vec4f pt{0.0f, 0.0f, 0.0f, 1.0f};
    return vec_eq((P * V * M) * pt, (gP * gV * gM) * glm::vec4{0, 0, 0, 1}, kEpsLoose);
}

static bool
mat4_mul_nonsequential()
{
    mat4f A{2, -1, 3, 0.5f, 0, 4, -2, 1, -3, 0, 1.5f, -0.5f, 1, 2, -1, 3};
    mat4f B{1.5f, -0.5f, 2, -1, 3, 1, -1.5f, 0, -2, 4, 0.5f, 2, 0.5f, -2, 1, 0.5f};
    glm::mat4 gA{glm::vec4{2, -1, 3, 0.5f},    glm::vec4{0, 4, -2, 1},
                 glm::vec4{-3, 0, 1.5f, -0.5f}, glm::vec4{1, 2, -1, 3}};
    glm::mat4 gB{glm::vec4{1.5f, -0.5f, 2, -1}, glm::vec4{3, 1, -1.5f, 0},
                 glm::vec4{-2, 4, 0.5f, 2},      glm::vec4{0.5f, -2, 1, 0.5f}};
    return mat_eq(A * B, gA * gB);
}

static bool
mat4_mul_vec4_nonsequential()
{
    mat4f A{2, -1, 3, 0.5f, 0, 4, -2, 1, -3, 0, 1.5f, -0.5f, 1, 2, -1, 3};
    glm::mat4 gA{glm::vec4{2, -1, 3, 0.5f},    glm::vec4{0, 4, -2, 1},
                 glm::vec4{-3, 0, 1.5f, -0.5f}, glm::vec4{1, 2, -1, 3}};
    vec4f v{1.0f, -1.0f, 2.0f, 0.5f};
    return vec_eq(A * v, gA * glm::vec4{1.0f, -1.0f, 2.0f, 0.5f});
}

static bool
mat4_tsr_30deg_y()
{
    mat4f M = translate(vec3f{1.0f, 0.0f, 0.0f})
            * scale(vec3f{2.0f, 2.0f, 2.0f})
            * rotate(0.5236f, vec3f{0.0f, 1.0f, 0.0f});
    glm::mat4 gM = glm::translate(glm::mat4(1.0f), glm::vec3{1, 0, 0})
                 * glm::scale(glm::mat4(1.0f), glm::vec3{2, 2, 2})
                 * glm::rotate(glm::mat4(1.0f), 0.5236f, glm::vec3{0, 1, 0});
    return mat_eq(M, gM);
}

static bool
mat4_tsr_90deg_diagonal()
{
    mat4f M = translate(vec3f{2.0f, 2.0f, -2.0f})
            * scale(vec3f{3.0f, 1.0f, 2.0f})
            * rotate(1.5708f, vec3f{1.0f, 1.0f, 0.0f});
    glm::mat4 gM = glm::translate(glm::mat4(1.0f), glm::vec3{2, 2, -2})
                 * glm::scale(glm::mat4(1.0f), glm::vec3{3, 1, 2})
                 * glm::rotate(glm::mat4(1.0f), 1.5708f, glm::vec3{1, 1, 0});
    return mat_eq(M, gM);
}

static bool
mat4_negate()
{
    mat4f A{3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3};
    glm::mat4 gA{glm::vec4{3, 1, 4, 1}, glm::vec4{5, 9, 2, 6},
                 glm::vec4{5, 3, 5, 8}, glm::vec4{9, 7, 9, 3}};
    return mat_eq(-A, -gA);
}

static bool
mat4_div_scalar()
{
    mat4f A{3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3};
    glm::mat4 gA{glm::vec4{3, 1, 4, 1}, glm::vec4{5, 9, 2, 6},
                 glm::vec4{5, 3, 5, 8}, glm::vec4{9, 7, 9, 3}};
    return mat_eq(A / 3.0f, gA / 3.0f);
}

static bool
mat4_sub_self_is_zero()
{
    mat4f A{3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3};
    mat4f diff = A - A;
    for (int i = 0; i < 16; ++i)
        if (!approx_eq(diff.elements[i], 0.0f)) return false;
    return true;
}

SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "default zero-init",             run_linear_test, LinearTestCase, mat4_zero_init);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "identity",                      run_linear_test, LinearTestCase, mat4_identity);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "diagonal constructor",          run_linear_test, LinearTestCase, mat4_diagonal_ctor);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "column-vec4 constructor",       run_linear_test, LinearTestCase, mat4_column_vec4_ctor);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "element access diagonal",       run_linear_test, LinearTestCase, mat4_element_access_diagonal);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "column access []",              run_linear_test, LinearTestCase, mat4_column_access);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "mat4 * scalar",                 run_linear_test, LinearTestCase, mat4_mul_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "scalar * mat4",                 run_linear_test, LinearTestCase, mat4_scalar_mul);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "mat4 + mat4",                   run_linear_test, LinearTestCase, mat4_add_mat4);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "mat4 - mat4",                   run_linear_test, LinearTestCase, mat4_sub_mat4);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "mat4 * mat4",                   run_linear_test, LinearTestCase, mat4_mul_mat4);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "mat4 *= mat4",                  run_linear_test, LinearTestCase, mat4_compound_mul_mat4);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "mat4 * identity (right)",       run_linear_test, LinearTestCase, mat4_mul_identity_right);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "identity * mat4 (left)",        run_linear_test, LinearTestCase, mat4_mul_identity_left);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "identity * vec4",               run_linear_test, LinearTestCase, mat4_mul_vec4_identity);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "scale * vec4",                  run_linear_test, LinearTestCase, mat4_mul_vec4_scale);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "transpose",                     run_linear_test, LinearTestCase, mat4_transpose);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "transpose(transpose) == self",  run_linear_test, LinearTestCase, mat4_double_transpose);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "translate matrix",              run_linear_test, LinearTestCase, mat4_translate);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "translate * point (w=1)",       run_linear_test, LinearTestCase, mat4_translate_point);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "translate * direction (w=0)",   run_linear_test, LinearTestCase, mat4_translate_direction);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "scale (vec3)",                  run_linear_test, LinearTestCase, mat4_scale_vec3);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "scale (uniform)",               run_linear_test, LinearTestCase, mat4_scale_uniform);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotate_x 45deg",                run_linear_test, LinearTestCase, mat4_rotate_x);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotate_y 60deg",                run_linear_test, LinearTestCase, mat4_rotate_y);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotate_z 30deg",                run_linear_test, LinearTestCase, mat4_rotate_z);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotate axis-angle (1,1,0)",     run_linear_test, LinearTestCase, mat4_rotate_axis_angle);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotation preserves length",     run_linear_test, LinearTestCase, mat4_rotation_preserves_length);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotations non-commutative",     run_linear_test, LinearTestCase, mat4_rotations_noncommutative);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotate_x identity at 0",        run_linear_test, LinearTestCase, mat4_rotate_x_identity);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotate_y identity at 0",        run_linear_test, LinearTestCase, mat4_rotate_y_identity);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotate_z identity at 0",        run_linear_test, LinearTestCase, mat4_rotate_z_identity);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotate_x 90deg",                run_linear_test, LinearTestCase, mat4_rotate_x_90deg);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotate_y 120deg",               run_linear_test, LinearTestCase, mat4_rotate_y_120deg);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotate_z 360deg",               run_linear_test, LinearTestCase, mat4_rotate_z_360deg);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotate 180deg around Y",        run_linear_test, LinearTestCase, mat4_rotate_180_around_y);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "rotate arbitrary axis",         run_linear_test, LinearTestCase, mat4_rotate_arbitrary_axis);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "perspective 45deg 16:9",        run_linear_test, LinearTestCase, mat4_perspective_45deg_16_9);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "perspective 60deg square",      run_linear_test, LinearTestCase, mat4_perspective_60deg_square);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "perspective 20deg 4:3",         run_linear_test, LinearTestCase, mat4_perspective_20deg_4_3);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "perspective deep frustum",      run_linear_test, LinearTestCase, mat4_perspective_deep_frustum);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "orthographic asymmetric",       run_linear_test, LinearTestCase, mat4_orthographic_asymmetric);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "orthographic unit cube",        run_linear_test, LinearTestCase, mat4_orthographic_unit_cube);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "orthographic screen-space",     run_linear_test, LinearTestCase, mat4_orthographic_screen_space);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "look_at from +Z",               run_linear_test, LinearTestCase, mat4_look_at_from_z);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "look_at non-trivial",           run_linear_test, LinearTestCase, mat4_look_at_nontrivial);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "look_at from +X",               run_linear_test, LinearTestCase, mat4_look_at_from_x);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "look_at offset center",         run_linear_test, LinearTestCase, mat4_look_at_offset_center);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "look_at long diagonal",         run_linear_test, LinearTestCase, mat4_look_at_long_diagonal);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "combined P*V*M matrix",         run_linear_test, LinearTestCase, mat4_combined_pvm_matrix);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "combined P*V*M * point",        run_linear_test, LinearTestCase, mat4_combined_pvm_point);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "mul non-sequential matrices",   run_linear_test, LinearTestCase, mat4_mul_nonsequential);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "mul non-sequential mat4*vec4",  run_linear_test, LinearTestCase, mat4_mul_vec4_nonsequential);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "T*S*R 30deg around Y",          run_linear_test, LinearTestCase, mat4_tsr_30deg_y);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "T*S*R 90deg diagonal axis",     run_linear_test, LinearTestCase, mat4_tsr_90deg_diagonal);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "unary negation",                run_linear_test, LinearTestCase, mat4_negate);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "mat4 / scalar",                 run_linear_test, LinearTestCase, mat4_div_scalar);
SIMPLEX_REGISTER_GROUPED_TEST("Linear: mat4", "A - A == zero",                 run_linear_test, LinearTestCase, mat4_sub_self_is_zero);

#endif // SIMPLEX_ENABLE_TESTS
