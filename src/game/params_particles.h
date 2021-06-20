// NOTE: This assumes unity build!
// NOTE: Using designated initializers in C++ is a GNU extension! Only compiles on clang and gcc
#pragma once
#include "game.h"

enum ParticleTypes {
    PT_None,
    PT_Test,

    PT_Count
};

enum ParticleEffectTypes {
    PE_None,
    PE_TestEmitter,

    PE_Count
};

#define KEYFRAMES_SCALAR(num) .count = num, .keys = (const ParticlePropKey_Scalar[num])
#define KEYFRAMES_VECTOR(num) .count = num, .keys = (const ParticlePropKey_Vector[num])

static const ParticleTemplate c_particle_templates[PT_Count] = {
    [PT_None] = {},
    [PT_Test] = {
        .lifetime = {0.5f, 2.0f},
        .spawn_dist = {1.0f, 5.0f},
        .speed = {
            KEYFRAMES_SCALAR(2) {
                {0.0f, 0.1f, 1.0f},
                {1.0f, -0.05f, 0.05f}
            }
        },
        .dir = {
            KEYFRAMES_VECTOR(1) {
                {0.0f, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}
            }
        },
        .size = {
            KEYFRAMES_VECTOR(2) {
                {0.0f, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
                {1.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}
            }
        }
    }
};
//static_assert(countof(c_particle_templates) < 256, "We gotta use uint16_t for template indices now!");

static const ParticleEffect c_particle_effects[PE_Count] = {
    [PE_None] = {},
    [PE_TestEmitter] = {
        .emitters = {
            {
                .template_idx = PT_Test,
                .spawn_rate = 0.05f,
                .lifetime = 2.0f
            }
        },
    }
};
