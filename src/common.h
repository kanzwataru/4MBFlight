#pragma once

/* common includes */
#include <stddef.h>
#include <stdint.h>

// TODO: replace assert with own macro!
#include <assert.h>

/* utility function macros */
#define STRINGIFY_B(x_) #x_
#define STRINGIFY(x_)   STRINGIFY_B(x_)
#define countof(x_) ((sizeof(x_) / sizeof(x_[0])))
#define STATIC_SIZEOF(type_, field_) (sizeof(((type_*)NULL)->field_))
#define STATIC_COUNTOF(type_, field_) ((sizeof(((type_*)NULL)->field_)) / (sizeof(((type_*)NULL)->field_[0])))

#define	KILOBYTES(x_) ((x_) * 1024)
#define MEGABYTES(x_) (KILOBYTES(x_) * 1024)
#define GIGABYTES(x_) (MEGABYTES(x_) * 1024)

/* compiler-specific */
#if defined(__GNUC__) || defined(__clang)
    #define ALIGN(x) __attribute__((aligned(x)))
    #define FORCEINLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
    #error "TODO: Support MSVC!"
#else
    #error "Unsupported compiler"
#endif
