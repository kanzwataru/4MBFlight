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
}

DEF_OP(+);
DEF_OP(-);
DEF_OP(*);
DEF_OP(/);

#undef DEF_OP

namespace math {

static inline float dot(v2 a, v2 b) {
    return a.x * b.x + a.y * b.y;
}

static inline float dot(v3 a, v3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline v2 normal(v2 v) {
    const float size_squared = dot(v, v);
    const float inv_sqrt = 1.0f / sqrtf(size_squared); // TODO PERF: Use intrinsic or enable fast math

    return { v.x * inv_sqrt, v.y * inv_sqrt };
}

static inline v3 cross(v3 a, v3 b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
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

}
