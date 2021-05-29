#pragma once
#include <stddef.h>
#include <stdint.h>

#define STRINGIFY_B(x_) #x_
#define STRINGIFY(x_)   STRINGIFY_B(x_)

#define	KILOBYTES(x_) ((x_) * 1024)
#define MEGABYTES(x_) (KILOBYTES(x_) * 1024)
#define GIGABYTES(x_) (MEGABYTES(x_) * 1024)
