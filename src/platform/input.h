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
    uint16_t transitions;
    uint16_t is_down;
};

struct AxisState {
    float delta;
    float value;
};

struct GameInputs {
    ButtonState fire;

    AxisState   roll_x;
    AxisState   roll_y;
    AxisState   pitch_x;
    AxisState   pitch_y;
    AxisState   yaw_x;
    AxisState   yaw_y;
    AxisState   throttle_relative;
};

/* helper functions */
inline bool button_down(ButtonState button)
{
    return button.is_down;
}

inline bool button_went_down(ButtonState button)
{
    return (button.transitions > 1) || (button.is_down && button.transitions == 1);
}

inline bool button_went_up(ButtonState button)
{
    return (button.transitions > 1) || (!button.is_down && button.transitions == 1);
}
