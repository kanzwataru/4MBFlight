/* 
 * Pre-processor defines to bridge the gap
 * between GLSL and C, allowing some (eg. struct) definitions
 * to be shared between the host program and shader code
 */
#pragma once
#ifdef C_CXX
#if 0
	#include "common.h"

	typedef ALIGN(16) float mat2x2[2][2];
	typedef ALIGN(16) float mat2x3[2][3];
	typedef ALIGN(16) float mat2x4[2][4];

	typedef ALIGN(16) float mat3x2[3][2];
	typedef ALIGN(16) float mat3x3[3][3];
	typedef ALIGN(16) float mat3x4[3][4];

	typedef ALIGN(16) float mat4x2[4][2];
	typedef ALIGN(16) float mat4x3[4][3];
	typedef ALIGN(16) float mat4x4[4][4];
	
	typedef mat2x2 mat2;
	typedef mat3x3 mat3;
	typedef mat4x4 mat4;

	typedef ALIGN(8)  float vec2[2];
	typedef ALIGN(16) float vec3[3];
	typedef ALIGN(16) float vec4[4];

	typedef ALIGN(8)  int32_t ivec2[2];
	typedef ALIGN(16) int32_t ivec3[3];
	typedef ALIGN(16) int32_t ivec4[4];

	typedef ALIGN(8)  uint32_t uvec2[2];
	typedef ALIGN(16) uint32_t uvec3[3];
	typedef ALIGN(16) uint32_t uvec4[4];
#else
    #include "game/mathlib.h"
    typedef ALIGN(8)  v2 vec2;
    typedef ALIGN(16) v3 vec3;
    typedef ALIGN(16) v4 vec4;

    typedef ALIGN(16) m44 mat4x4;

    typedef mat4x4 mat4;
    // TODO: Support all types!
#endif
#else /* GLSL */
	#define uint32_t uint
	#define int32_t int
#endif
