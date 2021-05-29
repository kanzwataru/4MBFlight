#include "globals.h"
#include "platform/platform.h"
#include "gpu.h"
#include "dev.h"

static void loaded(void *mem, const PlatformApi *api)
{
    g = (GlobalMemory *)mem;
    g->plf = api;

    gpu_loaded((void *)g->gpu_module, api->gl_get_proc_address);

    // TODO: Actual assets, for now assets is just the shaders
    auto *shader_storage = (ShaderStorageHeader *)api->assets;
    gpu_compile_shaders(g->shaders, shader_storage);

#if WITH_DEV
    dev_loaded();
#endif
}

static void init()
{
    gpu_init();
}

static void quit()
{
    gpu_quit();
}

static void update()
{
#if WITH_DEV
    dev_menu();
#endif
}

static void render()
{
    gpu_clear(0.15f, 0.25f, 0.30f, 1.0f);
}

extern "C" MODULE_GET_API_FUNC(MODULE_GET_API_NAME)
{
    constexpr size_t mem_size = GIGABYTES(1);
    static_assert(mem_size >= sizeof(GlobalMemory), "");

    api->mem_required = mem_size;

    api->loaded = loaded;
    api->init = init;
    api->quit = quit;
    api->update = update;
    api->render = render;
}
