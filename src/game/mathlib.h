#pragma once
#include "common.h"
#include <math.h>       // TODO: Don't use C stdlib

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
static FORCEINLINE void operator op##=(v2 a, const T b) {\
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
static FORCEINLINE void operator op##=(v3 a, const T b) {\
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
static FORCEINLINE void operator op##=(v4 a, const T b) {\
    a = a op b;\
}

DEF_OP(+);
DEF_OP(-);
DEF_OP(*);
DEF_OP(/);

#undef DEF_OP

static inline m44 operator*(const m44 &a, const m44 &b)
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

namespace math {

static inline float dot(v2 a, v2 b) {
    return a.x * b.x + a.y * b.y;
}

static inline float dot(v3 a, v3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <typename V>
static inline V normal(V v) {
    const float size_squared = dot(v, v);
    const float inv_sqrt = 1.0f / sqrtf(size_squared); // TODO PERF: Use intrinsic or enable fast math

    return v * inv_sqrt;
}

static inline v3 cross(v3 a, v3 b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static inline v3 append_axis(v2 v, float z)
{
    return {v.x, v.y, z};
}

static inline v4 append_axis(v3 v, float w)
{
    return {v.x, v.y, v.z, w};
}

static inline m44 m44_identity()
{
    return {
        {{1, 0, 0, 0},
         {0, 1, 0, 0},
         {0, 0, 1, 0},
         {0, 0, 0, 1}}
    };
}

static inline m44 proj_matrix_gl(float fov, float aspect_ratio, float near, float far)
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

}
