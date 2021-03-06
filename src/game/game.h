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

struct ParticlePropKey_Scalar {
    float normalized_time;
    float rand_min_value;
    float rand_max_value;
};

struct ParticlePropKey_Vector {
    float normalized_time;
    v3 rand_min_value;
    v3 rand_max_value;
};

struct ParticlePropTrack_Scalar {
    uint32_t count;
    const ParticlePropKey_Scalar *keys;
};

struct ParticlePropTrack_Vector {
    uint32_t count;
    const ParticlePropKey_Vector *keys;
};

struct ParticleTemplate {
    float lifetime[2];
    float spawn_dist[2];
    ParticlePropTrack_Scalar speed;
    ParticlePropTrack_Vector dir;   // TODO: This needs a better property than a v3
    ParticlePropTrack_Vector size;
    // ... more properties like type of particle, angular velocity, etc...
};

struct ParticleEmitter {
    uint32_t template_idx;
    float spawn_rate;
    int   num_per_spawn;
    float lifetime;

    // runtime, initialize to zero
    float spawn_counter;
};

struct ParticleEffect {
    ParticleEmitter emitters[32];
    m44 mat;
};

struct Particle {
    v3 pos;
    float lifetime;
    v3 rot;
    uint16_t template_idx;
    uint16_t rng_seed;
};
static_assert(sizeof(Particle) % 16 == 0, "");

struct Airplane {
    m44 mat;
    float velocity;
};

struct World {
    m44 view_mat;
    m44 proj_mat;

    Airplane player;
    bool has_player;

    Projectile projectiles[512]; // Tightly-packed array with unstable indices
    Particle particles[2048 * 16];  // Tightly-packed array with unstable indices
    ParticleEffect particle_effects[512]; // Tightly-packed array with unstable indices
};

struct Game {
    Mesh tri;
    Mesh uv_plane;
    Mesh cube;
    Pipeline pipeline_draw_sky;
    Pipeline pipeline_draw_flat;
    Pipeline pipeline_draw_grid;
    Pipeline pipeline_draw_lit;
    GPUBuffer flat_uniform;
    GPUBuffer lit_uniform;
    GPUBuffer sky_uniform;

    int res_width, res_height;

    uint64_t frame_number;
    bool paused;
    bool ejected;
};

static void particle_effect_spawn(const ParticleEffect *effect_template, v3 pos);

// --

struct GlobalMemory {
    const PlatformApi *plf;

    Game      game;
    World     world_game;
#if WITH_DEV
    World     world_particle_editor;
#endif
    World    *world;

    uint8_t gpu_module[GPU_MODULE_STATE_SIZE];

#if WITH_DEV
    uint8_t dev_module[DEV_MODULE_STATE_SIZE];
#endif

    StaticArena<MEGABYTES(32)>  permanent_mem;
    StaticArena<MEGABYTES(128)> scratch_mem;
};

static GlobalMemory *g;
