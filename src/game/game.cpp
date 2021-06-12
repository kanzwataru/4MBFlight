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

static float uv_plane_verts[] = {
    -1, -1, 0,  0, 0,
     1,  1, 0,  1, 1,
    -1,  1, 0,  0, 1,
     1, -1, 0,  1, 0
};

static uint16_t uv_plane_indices[] = {
    0, 1, 2, 0, 3, 1
};

static void init()
{
    gpu_init();

    g->game.view_mat = math::m44_identity();
    g->game.view_mat.m[3][1] = -1.0f;
    g->game.view_mat.m[3][2] = -2.0f;
    g->game.proj_mat = math::proj_matrix_gl(60.0f, float(g->plf->window_width) / float(g->plf->window_height), 0.01f, 100000.0f);

    g->game.flat_uniform.type = BT_Uniform;
    g->game.flat_uniform.size = sizeof(VertColUniform);
    gpu_buffer_add(&g->game.flat_uniform, nullptr);

    g->game.pipeline_draw_flat.shader = g->shaders[SP_VertCol];
    g->game.pipeline_draw_flat.uniforms[0] = &g->game.flat_uniform;
    gpu_pipeline_set(&g->game.pipeline_draw_flat);

    g->game.pipeline_draw_grid.shader = g->shaders[SP_Grid];
    g->game.pipeline_draw_grid.uniforms[0] = &g->game.flat_uniform;
    gpu_pipeline_set(&g->game.pipeline_draw_grid);

    g->game.tri.verts = tri_verts;
    g->game.tri.verts_count = countof(tri_verts);
    g->game.tri.layout = VL_PosCol;
    gpu_mesh_add(&g->game.tri);

    g->game.uv_plane.verts = uv_plane_verts;
    g->game.uv_plane.verts_count = countof(uv_plane_verts);
    g->game.uv_plane.indices = uv_plane_indices;
    g->game.uv_plane.indices_count = countof(uv_plane_indices);
    g->game.uv_plane.layout = VL_PosUV;
    gpu_mesh_add(&g->game.uv_plane);
}

static void quit()
{
    gpu_quit();
}

static void update(const UpdateInfo *upd)
{
#if WITH_DEV
    dev_menu();
    dev_rotate_cam(g->game.view_mat, upd);
#endif
}

static void render()
{
    gpu_clear(0.15f, 0.25f, 0.30f, 1.0f);

    VertColUniform uniform = {
        .model = {
            {{1, 0, 0, 0},
             {0, 1, 0, 0},
             {0, 0, 1, 0},
             {0, 1, 0, 1}}
        },
        //.view = apply_mat_translation(g->game.view_mat),
        .view = g->game.view_mat,
        .proj = g->game.proj_mat
    };

    gpu_buffer_update(&g->game.flat_uniform, &uniform);

    gpu_pipeline_set(&g->game.pipeline_draw_flat);
    gpu_mesh_draw(&g->game.tri);

    uniform.model = {
        {{1000, 0, 0, 0},
         {0, 0, 1000, 0},
         {0, 1000, 1000, 0},
         {0, 0, 0, 1}}
    };
    gpu_buffer_update(&g->game.flat_uniform, &uniform);

    gpu_pipeline_set(&g->game.pipeline_draw_grid);
    gpu_mesh_draw(&g->game.uv_plane);
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
