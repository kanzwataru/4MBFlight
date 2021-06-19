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
struct Projectile {
    v3 pos;
    v3 vel;
};

struct Game {
    m44 view_mat;
    m44 proj_mat;

    m44 cube_mat;
    float velocity;

    Projectile projectiles[512]; // Tightly-packed array with unstable indices

    Mesh tri;
    Mesh uv_plane;
    Mesh cube;
    Pipeline pipeline_draw_flat;
    Pipeline pipeline_draw_grid;
    Pipeline pipeline_draw_lit;
    GPUBuffer flat_uniform;
    GPUBuffer lit_uniform;

    int res_width, res_height;

    uint64_t frame_number;
    bool paused;
    bool ejected;
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
