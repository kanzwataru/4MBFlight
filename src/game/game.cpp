#include "globals.h"
#include "platform/platform.h"
#include "gpu.h"
#include "dev.h"
#include "shaders/struct_defs.h"

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

static float tri_verts[] = {
     1.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f,
     0.0f,  1.0f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0f,
};

static void init()
{
    gpu_init();

    g->game.flat_uniform.type = BT_Uniform;
    g->game.flat_uniform.size = sizeof(VertColUniform);
    gpu_buffer_add(&g->game.flat_uniform, nullptr);

    g->game.pipeline_draw_flat.shader = g->shaders[SP_VertCol];
    g->game.pipeline_draw_flat.uniforms[0] = &g->game.flat_uniform;
    gpu_pipeline_set(&g->game.pipeline_draw_flat);

    g->game.tri.verts = tri_verts;
    g->game.tri.verts_count = countof(tri_verts);
    g->game.tri.layout = VL_PosCol;
    gpu_mesh_add(&g->game.tri);
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

    VertColUniform uniform = {
        .model = {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
        },
        .view = {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
        },
        .proj = {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
        }
    };

    gpu_buffer_update(&g->game.flat_uniform, &uniform);

    gpu_pipeline_set(&g->game.pipeline_draw_flat);
    gpu_mesh_draw(&g->game.tri);
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
