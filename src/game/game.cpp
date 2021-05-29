#include "platform/platform.h"
#include "gpu.h"

// TEMP
#include "glad/glad.h"

static const PlatformApi *platform;

static void loaded(void *mem, const PlatformApi *api)
{
    platform = api;
#if WITH_HOTRELOAD
    gladLoadGLLoader(platform->gl_get_proc_address);
#endif
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
    glClearColor(0.15, 0.25, 0.65, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
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
