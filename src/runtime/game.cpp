#pragma once
#include "runtime/platform.h"

static Platform *g_platform;

static void loaded(Platform *plf)
{
    
}

static void init(void)
{
    
}

static void quit(void)
{
    
}

static void update_and_render(Event *events, int count)
{
    
}

extern "C" MODULE_GET_FUNC(MODULE_GET_FUNC_NAME)
{
    module->loaded = loaded;
    module->init = init;
    module->quit = quit;
    module->update_and_render = update_and_render;
}
