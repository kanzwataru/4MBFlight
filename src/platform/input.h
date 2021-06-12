#pragma once
#include "common.h"

#if WITH_DEV
struct Dev_InputState {
    float mouse[2];
    float mouse_rel[2];
    uint8_t mouse_button[3];
    uint8_t alt_key;
    uint8_t shift_key;
    uint8_t w, a, s, d;
};
#endif

struct ButtonState {
    uint8_t last_down;
    uint8_t down;
};

struct AxisState {
    float delta;
    float value;
};

struct GameInputs {
    ButtonState fire;

    AxisState   roll;
    AxisState   pitch;
    AxisState   yaw;
    AxisState   throttle_relative;
};
