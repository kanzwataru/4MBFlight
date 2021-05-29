// This assumes unity build!
#pragma once
#include "platform/platform.h"
#include "shaders/shader_toc.h"
#include "memory.h"
#include "gpu.h"

struct GlobalMemory {
    const PlatformApi *plf;

    Shader  shaders[SP_Total];
    uint8_t gpu_module[GPU_MODULE_STATE_SIZE];

    StaticArena<MEGABYTES(32)>  permanent_mem;
    StaticArena<MEGABYTES(128)> scratch_mem;
};

static GlobalMemory *g;
