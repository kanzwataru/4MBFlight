#pragma once
#include "common.h"

#if WITH_DEV
struct Dev_InputState {
    float mouse[2];
    float mouse_rel[2];
    uint8_t mouse_button[3];
    uint8_t alt_key;
    uint8_t shift_key;
};
#endif
