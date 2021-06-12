// This assumes unity build!
#pragma once
#include "platform/platform.h"
#include "shaders/shader_toc.h"
#include "memory.h"
#include "gpu.h"
#include "mathlib.h"

#if WITH_DEV
    #include "dev.h"
#endif

// -- TODO: Move this somewhere better mayhaps
struct Game {
    m44 view_mat;
    m44 proj_mat;

    Mesh tri;
    Mesh uv_plane;
    Pipeline pipeline_draw_flat;
    Pipeline pipeline_draw_grid;
    GPUBuffer flat_uniform;
};
// --

struct GlobalMemory {
    const PlatformApi *plf;

    Game    game;

    Shader  shaders[SP_Total];
    uint8_t gpu_module[GPU_MODULE_STATE_SIZE];

#if WITH_DEV
    uint8_t dev_module[DEV_MODULE_STATE_SIZE];
#endif

    StaticArena<MEGABYTES(32)>  permanent_mem;
    StaticArena<MEGABYTES(128)> scratch_mem;
};

static GlobalMemory *g;
