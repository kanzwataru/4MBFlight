#include "platform/platform.h"

static void loaded(void *mem)
{

}

static void init()
{

}

static void quit()
{

}

static void update()
{

}

static void render()
{

}

extern "C" MODULE_GET_API_FUNC(MODULE_GET_API_NAME)
{
    api->mem_required = GIGABYTES(1);

    api->loaded = loaded;
    api->init = init;
    api->quit = quit;
    api->update = update;
    api->render = render;
}
