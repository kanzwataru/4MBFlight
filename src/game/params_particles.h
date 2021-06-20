// NOTE: This assumes unity build!
// NOTE: Using designated initializers in C++ is a GNU extension! Only compiles on clang and gcc
#pragma once
#include "game.h"

enum ParticleTypes {
    PT_None,
    PT_Test,
    PT_TestLarge,

    PT_Count
};

#define PARTICLE_EFFECT_TYPES(X)\
    X(None)\
    X(TestEmitter)\
    X(TestB)

DEF_ENUM_CLASS(PARTICLE_EFFECT_TYPES, EffectTypes, int);
#if WITH_DEV
    DEF_STRING_LIST(PARTICLE_EFFECT_TYPES, c_effect_types_names);
#endif

#define KEYFRAMES_SCALAR(num) .count = num, .keys = (const ParticlePropKey_Scalar[num])
#define KEYFRAMES_VECTOR(num) .count = num, .keys = (const ParticlePropKey_Vector[num])

static const ParticleTemplate c_particle_templates[PT_Count] = {
    [PT_None] = {},
    [PT_Test] = {
        .lifetime = {0.25f, 2.0f},
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
                {0.0f, {0.1f, 0.1f, 0.1f}, {0.5f, 0.5f, 0.5f}},
                {1.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}
            }
        }
    },
    [PT_TestLarge] = {
        .lifetime = {2.0f, 6.0f},
        .spawn_dist = {1.0f, 5.0f},
        .speed = {
            KEYFRAMES_SCALAR(3) {
                {0.0f, 0.1f, 0.5f},
                {0.15f, 0.05f, 0.15f},
                {1.0f, -0.055f, 0.02f}
            }
        },
        .dir = {
            KEYFRAMES_VECTOR(2) {
                {0.0f, {0.2f, 0.75f, 0.2f}, {-0.3f, 0.75f, -0.3f}},
                {0.75f, {0.5f, 0.0f, 0.5f}, {-0.5f, 0.0f, -0.5f}},
            }
        },
        .size = {
            KEYFRAMES_VECTOR(5) {
                {0.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
                {0.05f, {0.05f, 0.25f, 0.05f}, {0.25f, 1.0f, 0.25f}},
                {0.25f, {2.0f, 2.0f, 2.0f}, {3.5f, 3.5f, 3.5f}},
                {0.55f, {3.0f, 3.0f, 3.0f}, {5.5f, 5.5f, 5.5f}},
                {1.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}}
            }
        }
    }
};
//static_assert(countof(c_particle_templates) < 256, "We gotta use uint16_t for template indices now!");

static const ParticleEffect c_particle_effects[(int)EffectTypes::Count] = {
    [(int)EffectTypes::None] = {},
    [(int)EffectTypes::TestEmitter] = {
        .emitters = {
            {
                .template_idx = PT_Test,
                .spawn_rate = 0.05f,
                .num_per_spawn = 1,
                .lifetime = 2.0f
            }
        },
    },
    [(int)EffectTypes::TestB] = {
        .emitters = {
            {
                .template_idx = PT_Test,
                .spawn_rate = 0.0f,
                .num_per_spawn = 15,
                .lifetime = 0.5f
            },
            {
                .template_idx = PT_TestLarge,
                .spawn_rate = 0.08f,
                .num_per_spawn = 3,
                .lifetime = 1.5f
            }
        }
    }
};
