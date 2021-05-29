#pragma once
#include "common.h"

struct PlatformApi {
	void *(*gl_get_proc_address)(const char *name);
};

struct ModuleApi {
    size_t mem_required;

    void (*loaded)(void *mem, const PlatformApi *api);
    void (*init)(void);
    void (*quit)(void);
    void (*update)();
    void (*render)(void);
};

#define MODULE_GET_API_NAME module_get_api
#define MODULE_GET_API_FUNC(name) void (name)(ModuleApi *api)
typedef MODULE_GET_API_FUNC(module_get_api_func_t);
