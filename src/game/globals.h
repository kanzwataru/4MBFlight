// This assumes unity build!
#pragma once
#include "platform/platform.h"
#include "shaders/shader_toc.h"
#include "memory.h"
#include "gpu.h"

// -- TODO: Move this somewhere better mayhaps
struct Game {
    Mesh tri;
    Pipeline pipeline_draw_flat;
    GPUBuffer flat_uniform;
};
// --

struct GlobalMemory {
    const PlatformApi *plf;

    Game    game;

    Shader  shaders[SP_Total];
    uint8_t gpu_module[GPU_MODULE_STATE_SIZE];

    StaticArena<MEGABYTES(32)>  permanent_mem;
    StaticArena<MEGABYTES(128)> scratch_mem;
};

static GlobalMemory *g;
