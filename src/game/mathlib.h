#pragma once
#include "common.h"
#include <math.h>       // TODO: Don't use C stdlib
#include <immintrin.h>

namespace mconst {
    constexpr float pi = M_PI;
    constexpr float tau = pi * 2.0f;
}

struct v2 {
    float x;
    float y;
};

struct v3 {
    float x;
    float y;
    float z;
};

struct v4 {
    float x;
    float y;
    float z;
    float w;
};

struct m44 {
    float m[4][4];
};

#define DEF_OP(op)\
static FORCEINLINE v2 operator op(const v2 a, const v2 b) {\
    return {a.x op b.x, a.y op b.y}; \
}\
\
template <typename T>\
static FORCEINLINE v2 operator op(const v2 a, const T b) {\
    return {a.x op b, a.y op b}; \
}\
\
template <typename T>\
static FORCEINLINE v2 operator op(const T a, const v2 b) {\
    return {a op b.x, a op b.y}; \
}\
\
template <typename T>\
static FORCEINLINE void operator op##=(v2 &a, const T b) {\
    a = a op b;\
}\
\
static FORCEINLINE v3 operator op(const v3 a, const v3 b) {\
    return {a.x op b.x, a.y op b.y, a.z op b.z}; \
}\
\
template <typename T>\
static FORCEINLINE v3 operator op(const v3 a, const T b) {\
    return {a.x op b, a.y op b, a.z op b}; \
}\
\
template <typename T>\
static FORCEINLINE v3 operator op(const T a, const v3 b) {\
    return {a op b.x, a op b.y, a op b.z}; \
}\
\
template <typename T>\
static FORCEINLINE void operator op##=(v3 &a, const T b) {\
    a = a op b;\
}\
\
static FORCEINLINE v4 operator op(const v4 a, const v4 b) {\
    return {a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w}; \
}\
\
template <typename T>\
static FORCEINLINE v4 operator op(const v4 a, const T b) {\
    return {a.x op b, a.y op b, a.z op b, a.w op b}; \
}\
\
template <typename T>\
static FORCEINLINE v4 operator op(const T a, const v4 b) {\
    return {a op b.x, a op b.y, a op b.z, a op b.w}; \
}\
\
template <typename T>\
static FORCEINLINE void operator op##=(v4 &a, const T b) {\
    a = a op b;\
}

DEF_OP(+);
DEF_OP(-);
DEF_OP(*);
DEF_OP(/);

#undef DEF_OP

inline v2 operator-(v2 v)
{
    return {-v.x, -v.y};
}

inline v3 operator-(v3 v)
{
    return {-v.x, -v.y, -v.z};
}

inline v4 operator-(v4 v)
{
    return {-v.x, -v.y, -v.z, -v.w};
}

inline m44 operator*(const m44 &a, const m44 &b)
{
    // from cglm
    m44 dest;
    dest.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[1][0] * b.m[0][1] + a.m[2][0] * b.m[0][2] + a.m[3][0] * b.m[0][3];
    dest.m[0][1] = a.m[0][1] * b.m[0][0] + a.m[1][1] * b.m[0][1] + a.m[2][1] * b.m[0][2] + a.m[3][1] * b.m[0][3];
    dest.m[0][2] = a.m[0][2] * b.m[0][0] + a.m[1][2] * b.m[0][1] + a.m[2][2] * b.m[0][2] + a.m[3][2] * b.m[0][3];
    dest.m[0][3] = a.m[0][3] * b.m[0][0] + a.m[1][3] * b.m[0][1] + a.m[2][3] * b.m[0][2] + a.m[3][3] * b.m[0][3];
    dest.m[1][0] = a.m[0][0] * b.m[1][0] + a.m[1][0] * b.m[1][1] + a.m[2][0] * b.m[1][2] + a.m[3][0] * b.m[1][3];
    dest.m[1][1] = a.m[0][1] * b.m[1][0] + a.m[1][1] * b.m[1][1] + a.m[2][1] * b.m[1][2] + a.m[3][1] * b.m[1][3];
    dest.m[1][2] = a.m[0][2] * b.m[1][0] + a.m[1][2] * b.m[1][1] + a.m[2][2] * b.m[1][2] + a.m[3][2] * b.m[1][3];
    dest.m[1][3] = a.m[0][3] * b.m[1][0] + a.m[1][3] * b.m[1][1] + a.m[2][3] * b.m[1][2] + a.m[3][3] * b.m[1][3];
    dest.m[2][0] = a.m[0][0] * b.m[2][0] + a.m[1][0] * b.m[2][1] + a.m[2][0] * b.m[2][2] + a.m[3][0] * b.m[2][3];
    dest.m[2][1] = a.m[0][1] * b.m[2][0] + a.m[1][1] * b.m[2][1] + a.m[2][1] * b.m[2][2] + a.m[3][1] * b.m[2][3];
    dest.m[2][2] = a.m[0][2] * b.m[2][0] + a.m[1][2] * b.m[2][1] + a.m[2][2] * b.m[2][2] + a.m[3][2] * b.m[2][3];
    dest.m[2][3] = a.m[0][3] * b.m[2][0] + a.m[1][3] * b.m[2][1] + a.m[2][3] * b.m[2][2] + a.m[3][3] * b.m[2][3];
    dest.m[3][0] = a.m[0][0] * b.m[3][0] + a.m[1][0] * b.m[3][1] + a.m[2][0] * b.m[3][2] + a.m[3][0] * b.m[3][3];
    dest.m[3][1] = a.m[0][1] * b.m[3][0] + a.m[1][1] * b.m[3][1] + a.m[2][1] * b.m[3][2] + a.m[3][1] * b.m[3][3];
    dest.m[3][2] = a.m[0][2] * b.m[3][0] + a.m[1][2] * b.m[3][1] + a.m[2][2] * b.m[3][2] + a.m[3][2] * b.m[3][3];
    dest.m[3][3] = a.m[0][3] * b.m[3][0] + a.m[1][3] * b.m[3][1] + a.m[2][3] * b.m[3][2] + a.m[3][3] * b.m[3][3];

    return dest;
}

inline v4 operator*(const m44 &m, v4 v)
{
    // from cglm
    return {
        m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0] * v.w,
        m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1] * v.w,
        m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2] * v.w,
        m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3] * v.w
    };
}

inline m44 operator*(m44 m, float s)
{
    m.m[0][0] *= s; m.m[0][1] *= s; m.m[0][2] *= s; m.m[0][3] *= s;
    m.m[1][0] *= s; m.m[1][1] *= s; m.m[1][2] *= s; m.m[1][3] *= s;
    m.m[2][0] *= s; m.m[2][1] *= s; m.m[2][2] *= s; m.m[2][3] *= s;
    m.m[3][0] *= s; m.m[3][1] *= s; m.m[3][2] *= s; m.m[3][3] *= s;

    return m;
}

namespace math {

template <typename T>
inline T min(T a, T b) {
    return a < b ? a : b;
}

template <typename T>
inline T max(T a, T b) {
    return a > b ? a : b;
}

template <typename T>
inline T clamp(T v, T min_val, T max_val) {
    return v < min_val ? min_val : (v > max_val ? max_val : v);
}

template <typename VT, typename TT>
inline VT lerp(VT a, VT b, TT t) {
    return a + t * (b - a);
}

inline float dot(v2 a, v2 b) {
    return a.x * b.x + a.y * b.y;
}

inline float dot(v3 a, v3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <typename V>
inline V normal(V v) {
    const float size_squared = dot(v, v);
    const float inv_sqrt = 1.0f / sqrtf(size_squared); // TODO PERF: Use intrinsic or enable fast math

    return v * inv_sqrt;
}

inline v3 cross(v3 a, v3 b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

inline v3 append_axis(v2 v, float z)
{
    return {v.x, v.y, z};
}

inline v4 append_axis(v3 v, float w)
{
    return {v.x, v.y, v.z, w};
}

inline float deg_to_rad(float deg)
{
    return (deg * mconst::pi) / 180.0f;
}

inline float rad_to_deg(float rad)
{
    return (rad * 180.0f) / mconst::pi;
}

inline constexpr m44 m44_identity()
{
    return {
        {{1, 0, 0, 0},
         {0, 1, 0, 0},
         {0, 0, 1, 0},
         {0, 0, 0, 1}}
    };
}

inline m44 proj_matrix_gl(float fov, float aspect_ratio, float near, float far)
{
    /* https://solarianprogrammer.com/2013/05/22/opengl-101-matrices-projection-view-model/
     * https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/opengl-perspective-projection-matrix
     */
    float top = near * tan(fov * 0.5f * M_PI / 180.0f);
    float bottom = -top;
    float right = top * aspect_ratio;
    float left = -right;

#if 0
    return {{
        {(2.0f * near) / (right - left), 0.0f, (right + left) / (right - left), 0.0f},
        {0.0f, (2.0f * near) / (top - bottom), (top + bottom) / (top - bottom), 0.0f},
        {0.0f, 0.0f, -((far + near) / (far - near)), -((2 * far * near) / (far - near))},
        {0.0f, 0.0f, -1.0f, 0.0f}
    }};
#else
    m44 ret;
    ret.m[0][0] = (2.0f * near) / (right - left);
    ret.m[0][1] = 0.0f;
    ret.m[0][2] = 0.0f;
    ret.m[0][3] = 0.0f;

    ret.m[1][0] = 0.0f;
    ret.m[1][1] = (2.0f * near) / (top - bottom);
    ret.m[1][2] = 0.0f;
    ret.m[1][3] = 0.0f;

    ret.m[2][0] = (right + left) / (right - left);
    ret.m[2][1] = (top + bottom) / (top - bottom);
    ret.m[2][2] = -(far + near) / (far - near);
    ret.m[2][3] = -1.0f;

    ret.m[3][0] = 0.0f;
    ret.m[3][1] = 0.0f;
    ret.m[3][2] = (-2.0f * far * near) / (far - near);
    ret.m[3][3] = 0.0f;
    return ret;
#endif
}

inline v3 v3_from_axis(const m44 &mat, int i)
{
    return {mat.m[i][0], mat.m[i][1], mat.m[i][2]};
}

inline m44 v3_to_axis(m44 mat, int i, v3 v)
{
    mat.m[i][0] = v.x;
    mat.m[i][1] = v.y;
    mat.m[i][2] = v.z;
    return mat;
}

inline m44 make_translate_matrix(v3 translation)
{
    return {
        {{1, 0, 0, 0},
         {0, 1, 0, 0},
         {0, 0, 1, 0},
         {translation.x, translation.y, translation.z, 1.0f}}
    };
}

inline m44 make_rot_matrix(v3 axis, float angle)
{
    /* from cglm */
    m44 m = math::m44_identity();
    float c = cosf(angle);

    v3 axisn = normal(axis);
    v3 v = axisn * (1.0f - c);
    v3 vs = axisn * sinf(angle);

    v3 m0 = axisn * v.x;
    v3 m1 = axisn * v.y;
    v3 m2 = axisn * v.z;

    m.m[0][0] = m0.x;
    m.m[0][1] = m0.y;
    m.m[0][2] = m0.z;

    m.m[1][0] = m1.x;
    m.m[1][1] = m1.y;
    m.m[1][2] = m1.z;

    m.m[2][0] = m2.x;
    m.m[2][1] = m2.y;
    m.m[2][2] = m2.z;

    m.m[0][0] += c;       m.m[1][0] -= vs.z;   m.m[2][0] += vs.y;
    m.m[0][1] += vs.z;   m.m[1][1] += c;       m.m[2][1] -= vs.x;
    m.m[0][2] -= vs.y;   m.m[1][2] += vs.x;   m.m[2][2] += c;

    m.m[0][3] = m.m[1][3] = m.m[2][3] = m.m[3][0] = m.m[3][1] = m.m[3][2] = 0.0f;
    m.m[3][3] = 1.0f;

    return m;
}

inline m44 inverse(m44 mat)
{
    /* from cglm */
    m44 dest;

    float t[6];
    float det;
    float a = mat.m[0][0], b = mat.m[0][1], c = mat.m[0][2], d = mat.m[0][3],
          e = mat.m[1][0], f = mat.m[1][1], g = mat.m[1][2], h = mat.m[1][3],
          i = mat.m[2][0], j = mat.m[2][1], k = mat.m[2][2], l = mat.m[2][3],
          m = mat.m[3][0], n = mat.m[3][1], o = mat.m[3][2], p = mat.m[3][3];

    t[0] = k * p - o * l; t[1] = j * p - n * l; t[2] = j * o - n * k;
    t[3] = i * p - m * l; t[4] = i * o - m * k; t[5] = i * n - m * j;

    dest.m[0][0] =  f * t[0] - g * t[1] + h * t[2];
    dest.m[1][0] =-(e * t[0] - g * t[3] + h * t[4]);
    dest.m[2][0] =  e * t[1] - f * t[3] + h * t[5];
    dest.m[3][0] =-(e * t[2] - f * t[4] + g * t[5]);

    dest.m[0][1] =-(b * t[0] - c * t[1] + d * t[2]);
    dest.m[1][1] =  a * t[0] - c * t[3] + d * t[4];
    dest.m[2][1] =-(a * t[1] - b * t[3] + d * t[5]);
    dest.m[3][1] =  a * t[2] - b * t[4] + c * t[5];

    t[0] = g * p - o * h; t[1] = f * p - n * h; t[2] = f * o - n * g;
    t[3] = e * p - m * h; t[4] = e * o - m * g; t[5] = e * n - m * f;

    dest.m[0][2] =  b * t[0] - c * t[1] + d * t[2];
    dest.m[1][2] =-(a * t[0] - c * t[3] + d * t[4]);
    dest.m[2][2] =  a * t[1] - b * t[3] + d * t[5];
    dest.m[3][2] =-(a * t[2] - b * t[4] + c * t[5]);

    t[0] = g * l - k * h; t[1] = f * l - j * h; t[2] = f * k - j * g;
    t[3] = e * l - i * h; t[4] = e * k - i * g; t[5] = e * j - i * f;

    dest.m[0][3] =-(b * t[0] - c * t[1] + d * t[2]);
    dest.m[1][3] =  a * t[0] - c * t[3] + d * t[4];
    dest.m[2][3] =-(a * t[1] - b * t[3] + d * t[5]);
    dest.m[3][3] =  a * t[2] - b * t[4] + c * t[5];

    det = 1.0f / (a * dest.m[0][0] + b * dest.m[1][0]
                + c * dest.m[2][0] + d * dest.m[3][0]);

    return dest * det;
}

inline v3 euler_from_mat(m44 m)
{
    /* from cglm */
    float m00, m01, m10, m11, m20, m21, m22;
    float thetaX, thetaY, thetaZ;

    m00 = m.m[0][0];  m10 = m.m[1][0];  m20 = m.m[2][0];
    m01 = m.m[0][1];  m11 = m.m[1][1];  m21 = m.m[2][1];
                                    m22 = m.m[2][2];

    if (m20 < 1.0f) {
      if (m20 > -1.0f) {
        thetaY = asinf(m20);
        thetaX = atan2f(-m21, m22);
        thetaZ = atan2f(-m10, m00);
      } else { /* m20 == -1 */
        /* Not a unique solution */
        thetaY = -mconst::tau;
        thetaX = -atan2f(m01, m11);
        thetaZ =  0.0f;
      }
    } else { /* m20 == +1 */
      thetaY = -mconst::tau;
      thetaX = atan2f(m01, m11);
      thetaZ = 0.0f;
    }

    return {
        thetaX,
        thetaY,
        thetaZ
    };
}

inline m44 normalize_rot_axes(m44 mat)
{
    mat = v3_to_axis(mat, 0, normal(v3_from_axis(mat, 0)));
    mat = v3_to_axis(mat, 1, normal(v3_from_axis(mat, 1)));
    mat = v3_to_axis(mat, 2, normal(v3_from_axis(mat, 2)));
    return mat;
}

}
