#pragma once
#include "common.h"

struct Platform {
    //void *(*mem_alloc)(size_t size);
    void (*mem_free)(void);
};

struct Event {
    
};

struct Module {
    void (*loaded)(Platform *plf);
    void (*init)(void);
    void (*quit)(void);
    void (*update_and_render)(Event *events, int count);
};

#define MODULE_GET_FUNC_NAME  module_get
#define MODULE_GET_FUNC(name) void(name)(Module *module)
typedef MODULE_GET_FUNC(module_get_func_t);
