#pragma once
#include "common.h"
#include "input.h"

struct PlatformApi {
	void *(*gl_get_proc_address)(const char *name);
    void *dear_imgui_ctx;

    uint8_t *assets;
    size_t   assets_size;

    int window_width; // TODO: Move these to an event
    int window_height;
};

struct PlatformOptions {
    bool lock_mouse;
    bool reset_axes_next_frame;
};

struct UpdateInfo {
    float delta_time;

    GameInputs input;
#if WITH_DEV
    Dev_InputState devinput_prev;
    Dev_InputState devinput;
#endif
};

struct ModuleApi {
    size_t mem_required;

    void (*loaded)(void *mem, const PlatformApi *api);
    void (*init)(PlatformOptions *options);
    void (*quit)(void);
    void (*update)(const UpdateInfo *upd, PlatformOptions *options);
    void (*render)(void);
};

#define MODULE_GET_API_NAME module_get_api
#define MODULE_GET_API_FUNC(name) void (name)(ModuleApi *api)
typedef MODULE_GET_API_FUNC(module_get_api_func_t);
