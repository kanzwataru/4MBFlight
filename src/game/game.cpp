// TODO: Use delta_time!!!
#include "game.h"
#include "primitives.h"
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

static void viewport_sized(int width, int height)
{
    g->game.proj_mat = math::proj_matrix_gl(60.0f, float(width) / float(height), 0.01f, 100000.0f);
    gpu_viewport_set(0, 0, width, height);
}

static void init(PlatformOptions *options)
{
    gpu_init();
#if WITH_DEV
    dev_init();
    g->game.paused = true;
    g->game.ejected = true;
#endif

    options->lock_mouse = true;
    g->game.res_width = g->plf->window_width;
    g->game.res_height = g->plf->window_height;

    // rendering state
    g->game.flat_uniform.type = BT_Uniform;
    g->game.flat_uniform.size = sizeof(VertColUniform);
    gpu_buffer_add(&g->game.flat_uniform, nullptr);

    g->game.lit_uniform.type = BT_Uniform;
    g->game.lit_uniform.size = sizeof(VertColUniform);
    gpu_buffer_add(&g->game.lit_uniform, nullptr);

    g->game.pipeline_draw_flat.shader = g->shaders[SP_VertCol];
    g->game.pipeline_draw_flat.uniforms[0] = &g->game.flat_uniform;

    g->game.pipeline_draw_grid.shader = g->shaders[SP_Grid];
    g->game.pipeline_draw_grid.alpha_blending = true;
    g->game.pipeline_draw_grid.no_depth_write = true;
    g->game.pipeline_draw_grid.uniforms[0] = &g->game.flat_uniform;

    g->game.pipeline_draw_lit.shader = g->shaders[SP_Lit];
    g->game.pipeline_draw_lit.uniforms[0] = &g->game.lit_uniform;

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

    g->game.cube.verts = cube_verts;
    g->game.cube.verts_count = countof(cube_verts);
    g->game.cube.layout = VL_PosNormUV;
    gpu_mesh_add(&g->game.cube);

    // game state
    g->game.view_mat = math::m44_identity();
    g->game.view_mat.m[3][1] = -1.0f;
    g->game.view_mat.m[3][2] = -2.0f;

    viewport_sized(g->game.res_width, g->game.res_height);

    g->game.cube_mat = {
        {{1, 0, 0, 0},
         {0, 1, 0, 0},
         {0, 0, 1, 0},
         {0, 0, 0, 1}}
    };
}

static void quit()
{
    gpu_quit();
}

static void projectile_spawn(v3 pos, v3 dir)
{
    auto *proj = packed_array_add(g->game.projectiles);
    if(!proj)
        return;

    proj->pos = pos;
    proj->vel = dir * 15.0f;
}

static void update_projectiles(const UpdateInfo *upd)
{
    (void)upd;

    uint32_t destroy_list[countof(g->game.projectiles)];
    uint32_t destroy_count = 0;

    packed_array_iterate(g->game.projectiles, [&](uint32_t i) {
        auto *proj = &g->game.projectiles[i];
        proj->pos += proj->vel;
        proj->pos.y -= 0.01f;
        proj->vel *= 0.99999f;

        // dummy floor collision
        if(proj->pos.y < 0.0f) {
            destroy_list[destroy_count++] = i;
        }
    });

    packed_array_remove(g->game.projectiles, destroy_list, destroy_count);
}

static void update_airplane(const UpdateInfo *upd)
{
    v3 mrot = math::euler_from_mat(g->game.cube_mat);
#if WITH_DEV
    ImGui::Begin("Info");
    ImGui::LabelText("Rot from cube_mat", "%f %f %f", mrot.x, mrot.y, mrot.z);
    ImGui::LabelText("Pos from cube_mat", "%f %f %f", g->game.cube_mat.m[3][0], g->game.cube_mat.m[3][1], g->game.cube_mat.m[3][2]);
    ImGui::End();
#endif
    // - Flight physics
    constexpr float max_velocity = 1.5f;
    constexpr float gravity = -0.3f;

    const float pitch_input = -upd->input.pitch.value * 0.027f;
    const float yaw_input = -upd->input.yaw.value * 0.005f;
    const float roll_input = -upd->input.roll.value * 0.015f;
    const float throttle_input = upd->input.throttle.value;

    const float velocity_percent = g->game.velocity / max_velocity;
    const float turn_yaw_force = sinf(mrot.z * 2.0f) * 0.0025f;
    const float control_force_multiplier = velocity_percent;
    const float thrust_force = throttle_input * 0.02f;
    const float drag_force = (1.0f - throttle_input) * 0.005f;

    const float pitch_delta = pitch_input * control_force_multiplier;
    const float yaw_delta = yaw_input * control_force_multiplier + turn_yaw_force;
    const float roll_delta = roll_input * control_force_multiplier;
    const float gravity_force = gravity * (1.0f - velocity_percent);
    const float lift_force = velocity_percent * 0.01f;
    const float vertical_force = lift_force + gravity_force;

    g->game.velocity = math::clamp(g->game.velocity + thrust_force - drag_force, 0.0f, max_velocity);

    m44 pos_matrix = math::make_translate_matrix({0.0f, vertical_force, -g->game.velocity});
    m44 rot_matrix = math::m44_identity();
    rot_matrix = rot_matrix * math::make_rot_matrix({1.0f, 0.0f, 0.0f}, pitch_delta);
    rot_matrix = rot_matrix * math::make_rot_matrix({0.0f, 0.0f, 1.0f}, roll_delta);
    rot_matrix = rot_matrix * math::make_rot_matrix({0.0f, 1.0f, 0.0f}, yaw_delta);

    m44 delta_matrix = pos_matrix * rot_matrix;

    g->game.cube_mat = g->game.cube_mat * delta_matrix;
    g->game.cube_mat.m[3][1] = math::max(0.0f, g->game.cube_mat.m[3][1]);

    // - Projectile firing
    if(upd->input.fire.down && !upd->input.fire.last_down) {
        projectile_spawn(math::v3_from_axis(g->game.cube_mat, 3),
                         -math::v3_from_axis(g->game.cube_mat, 2));
    }
}

static void update(const UpdateInfo *upd, PlatformOptions *options)
{
    if(g->plf->window_width != g->game.res_width || g->plf->window_height != g->game.res_height) {
        g->game.res_width = g->plf->window_width;
        g->game.res_height = g->plf->window_height;
        viewport_sized(g->game.res_width, g->game.res_height);
    }

#if WITH_DEV
    dev_menu(upd, options);

    if(g->game.ejected) {
        dev_rotate_cam(g->game.view_mat, upd);
    }    
#endif
    options->lock_mouse = !g->game.paused;

    if(!g->game.paused || g->game.frame_number == 0) {       
        update_airplane(upd);
        update_projectiles(upd);

        g->game.frame_number++;
    }

    if(!g->game.ejected) {
        g->game.view_mat = math::inverse(g->game.cube_mat * math::make_translate_matrix({0.0f, 2.0f, 5.0f}));
    }
}

static void render()
{
    gpu_clear(0.15f, 0.25f, 0.30f, 1.0f);

    m44 view_mat_inv = g->game.view_mat;

    VertColUniform uniform = {
        .model = {
            {{1, 0, 0, 0},
             {0, 1, 0, 0},
             {0, 0, 1, 0},
             {0, 1, 0, 1}}
        },
        .view = view_mat_inv,
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

    LitUniform lit_uniform = {
        .model = g->game.cube_mat,
        .view = g->game.view_mat,
        .proj = g->game.proj_mat
    };

    gpu_buffer_update(&g->game.lit_uniform, &lit_uniform);
    gpu_pipeline_set(&g->game.pipeline_draw_lit);

    gpu_mesh_draw(&g->game.cube); // "airplane"

    packed_array_iterate(g->game.projectiles, [&](uint32_t i) {
        const auto &proj = g->game.projectiles[i];
        lit_uniform.model = math::make_translate_matrix(proj.pos);

        gpu_buffer_update(&g->game.lit_uniform, &lit_uniform);
        gpu_mesh_draw(&g->game.cube);
    });
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
