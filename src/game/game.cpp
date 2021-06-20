// TODO: Use delta_time!!!
#include "game.h"
#include "primitives.h"
#include "platform/platform.h"
#include "gpu.h"
#include "dev.h"
#include "shaders/struct_defs.h"
#include "params_particles.h"
#include "randgen.h"

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
    g->world->proj_mat = math::proj_matrix_gl(60.0f, float(width) / float(height), 0.01f, 100000.0f);
    gpu_viewport_set(0, 0, width, height);
}

static void init(PlatformOptions *options)
{
    gpu_init();

    g->world = &g->game_world;
    g->world->has_player = true;
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
    g->world->view_mat = math::m44_identity();
    g->world->view_mat.m[3][1] = -1.0f;
    g->world->view_mat.m[3][2] = -2.0f;

    viewport_sized(g->game.res_width, g->game.res_height);

    g->world->player.mat = {
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

static void particle_effect_spawn(const ParticleEffect *effect_template, v3 pos)
{
    auto *eff = packed_array_add(g->world->particle_effects);
    if(!eff)
        return;

    *eff = *effect_template;
    eff->mat = math::make_translate_matrix(pos);
}

static void particle_spawn(v3 pos, uint16_t template_idx)
{
    auto *prt = packed_array_add(g->world->particles);
    if(!prt)
        return;

    *prt = {};
    prt->template_idx = template_idx;
    prt->pos = pos;
    prt->rng_seed = uint16_t(rand_xorshift_32() >> 4);
}

inline float keyframe_sample(const ParticlePropTrack_Scalar &track, float t, uint16_t rng_val)
{
    assert(track.count > 0);

    for(uint32_t i = 0; i < track.count; ++i) {
        if(track.keys[i].normalized_time > t) {
            if(i == 0) {
                break;
            }
            else {
                auto &prev = track.keys[i - 1];
                auto &next = track.keys[i];

                assert(prev.normalized_time < next.normalized_time);
                float interp_t = (t - prev.normalized_time) / (next.normalized_time - prev.normalized_time);

                return remap_to_float(math::lerp(prev.rand_min_value, next.rand_min_value, interp_t),
                                      math::lerp(prev.rand_max_value, next.rand_max_value, interp_t), rng_val);
            }
        }
    }

    return remap_to_float(track.keys[0].rand_min_value, track.keys[0].rand_max_value, rng_val);
}

inline v3 keyframe_sample(const ParticlePropTrack_Vector &track, float t, uint16_t rng_val)
{
    assert(track.count > 0);

    for(uint32_t i = 0; i < track.count; ++i) {
        if(track.keys[i].normalized_time > t) {
            if(i == 0) {
                break;
            }
            else {
                auto &prev = track.keys[i - 1];
                auto &next = track.keys[i];

                assert(prev.normalized_time < next.normalized_time);
                float interp_t = (t - prev.normalized_time) / (next.normalized_time - prev.normalized_time);

                return {
                    remap_to_float(math::lerp(prev.rand_min_value.x, next.rand_min_value.x, interp_t),
                                   math::lerp(prev.rand_max_value.x, next.rand_max_value.x, interp_t), rng_val),
                    remap_to_float(math::lerp(prev.rand_min_value.y, next.rand_min_value.y, interp_t),
                                   math::lerp(prev.rand_max_value.y, next.rand_max_value.y, interp_t), rng_val),
                    remap_to_float(math::lerp(prev.rand_min_value.z, next.rand_min_value.z, interp_t),
                                   math::lerp(prev.rand_max_value.z, next.rand_max_value.z, interp_t), rng_val),
                };
            }
        }
    }

    return {
        remap_to_float(track.keys[0].rand_min_value.x, track.keys[0].rand_max_value.x, rng_val),
        remap_to_float(track.keys[0].rand_min_value.y, track.keys[0].rand_max_value.y, rng_val),
        remap_to_float(track.keys[0].rand_min_value.z, track.keys[0].rand_max_value.z, rng_val),
    };
}

static void update_particles(const UpdateInfo *upd)
{
    uint32_t effect_destroy_list[countof(g->world->particle_effects)];
    uint32_t effect_destroy_count = 0;

    uint32_t particle_destroy_list[countof(g->world->particles)];
    uint32_t particle_destroy_count = 0;

    // Particle emitters
    packed_array_iterate(g->world->particle_effects, [&](uint32_t i) {
        auto *eff = &g->world->particle_effects[i];

        int emitter_count = 0;
        int alive_emitter_count = 0;

        for(auto &emitter : eff->emitters) {
            if(emitter.template_idx == 0)
                break;

            ++emitter_count;
            emitter.lifetime -= upd->delta_time;
            if(emitter.lifetime > 0.0f) {
                ++alive_emitter_count;

                emitter.spawn_counter += upd->delta_time;
                if(emitter.spawn_counter > emitter.spawn_rate) {
                    emitter.spawn_counter -= emitter.spawn_rate;

                    particle_spawn(math::v3_from_axis(eff->mat, 3), emitter.template_idx);
                }
            }
            else {
                emitter.lifetime = 0.0f;
            }
        }

        if(alive_emitter_count == 0 || emitter_count == 0) {
            effect_destroy_list[effect_destroy_count++] = i;
        }
    });

    // Particles
    packed_array_iterate(g->world->particles, [&](uint32_t i) {
        auto *prt = &g->world->particles[i];
        const auto &templ = c_particle_templates[prt->template_idx];

        const float total_lifetime = remap_to_float(templ.lifetime[0], templ.lifetime[1], prt->rng_seed);

        prt->lifetime += upd->delta_time;
        if(prt->lifetime < total_lifetime) {
            const float t = prt->lifetime / total_lifetime;

            const float speed = keyframe_sample(templ.speed, t, prt->rng_seed);
            const v3 dir = math::normal(keyframe_sample(templ.dir, t, prt->rng_seed));

            prt->pos += dir * speed;
        }
        else {
            particle_destroy_list[particle_destroy_count++] = i;
        }
    });

    packed_array_remove(g->world->particles, particle_destroy_list, particle_destroy_count);
    packed_array_remove(g->world->particle_effects, effect_destroy_list, effect_destroy_count);
}

static void projectile_spawn(v3 pos, v3 dir)
{
    auto *proj = packed_array_add(g->world->projectiles);
    if(!proj)
        return;

    *proj = {};
    proj->pos = pos;
    proj->vel = dir * 10.0f;
}

static void update_projectiles(const UpdateInfo *upd)
{
    (void)upd;

    uint32_t destroy_list[countof(g->world->projectiles)];
    uint32_t destroy_count = 0;

    packed_array_iterate(g->world->projectiles, [&](uint32_t i) {
        auto *proj = &g->world->projectiles[i];
        proj->pos += proj->vel;
        proj->pos.y -= 0.01f;
        proj->vel *= 0.99999f;

        // dummy floor collision
        if(proj->pos.y < 0.0f) {
            destroy_list[destroy_count++] = i;
            particle_effect_spawn(&c_particle_effects[PE_TestEmitter], proj->pos);
        }
    });

    packed_array_remove(g->world->projectiles, destroy_list, destroy_count);
}

static void update_airplane(struct Airplane *plane, const UpdateInfo *upd)
{
    v3 mrot = math::euler_from_mat(plane->mat);
#if WITH_DEV
    ImGui::Begin("Info");
    ImGui::LabelText("Rot from cube_mat", "%f %f %f", mrot.x, mrot.y, mrot.z);
    ImGui::LabelText("Pos from cube_mat", "%f %f %f", plane->mat.m[3][0], plane->mat.m[3][1], plane->mat.m[3][2]);
    ImGui::End();
#endif
    // - Flight physics
    constexpr float max_velocity = 1.5f;
    constexpr float gravity = -0.3f;

    const float pitch_input = -upd->input.pitch.value * 0.027f;
    const float yaw_input = -upd->input.yaw.value * 0.005f;
    const float roll_input = -upd->input.roll.value * 0.015f;
    const float throttle_input = upd->input.throttle.value;

    const float velocity_percent = plane->velocity / max_velocity;
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

    plane->velocity = math::clamp(plane->velocity + thrust_force - drag_force, 0.0f, max_velocity);

    m44 pos_matrix = math::make_translate_matrix({0.0f, vertical_force, -plane->velocity});
    m44 rot_matrix = math::m44_identity();
    rot_matrix = rot_matrix * math::make_rot_matrix({1.0f, 0.0f, 0.0f}, pitch_delta);
    rot_matrix = rot_matrix * math::make_rot_matrix({0.0f, 0.0f, 1.0f}, roll_delta);
    rot_matrix = rot_matrix * math::make_rot_matrix({0.0f, 1.0f, 0.0f}, yaw_delta);

    m44 delta_matrix = pos_matrix * rot_matrix;

    plane->mat = plane->mat * delta_matrix;
    plane->mat.m[3][1] = math::max(0.0f, plane->mat.m[3][1]);

    // - Projectile firing
    if(upd->input.fire.down && !upd->input.fire.last_down) {
        projectile_spawn(math::v3_from_axis(plane->mat, 3),
                         -math::v3_from_axis(plane->mat, 2));
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
        dev_rotate_cam(g->world->view_mat, upd);
    }    
#endif
    options->lock_mouse = !g->game.paused;

    if(!g->game.paused || g->game.frame_number == 0) {
        if(g->world->has_player) {
            update_airplane(&g->world->player, upd);
        }
        update_projectiles(upd);
        update_particles(upd);

        g->game.frame_number++;
    }

    if(!g->game.ejected) {
        g->world->view_mat = math::inverse(g->world->player.mat * math::make_translate_matrix({0.0f, 2.0f, 5.0f}));
    }
}

static void render()
{
    gpu_clear(0.15f, 0.25f, 0.30f, 1.0f);

    m44 view_mat_inv = g->world->view_mat;

    VertColUniform uniform = {
        .model = {
            {{1, 0, 0, 0},
             {0, 1, 0, 0},
             {0, 0, 1, 0},
             {0, 1, 0, 1}}
        },
        .view = view_mat_inv,
        .proj = g->world->proj_mat
    };

    gpu_buffer_update(&g->game.flat_uniform, &uniform);

    gpu_pipeline_set(&g->game.pipeline_draw_flat);
    if(g->world->has_player) {
        // Only draw debug tri when we have a player
        gpu_mesh_draw(&g->game.tri);
    }

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
        .model = g->world->player.mat,
        .view = g->world->view_mat,
        .proj = g->world->proj_mat
    };

    gpu_buffer_update(&g->game.lit_uniform, &lit_uniform);
    gpu_pipeline_set(&g->game.pipeline_draw_lit);

    if(g->world->has_player) {
        gpu_mesh_draw(&g->game.cube); // "airplane"
    }

    packed_array_iterate(g->world->projectiles, [&](uint32_t i) {
        const auto *proj = &g->world->projectiles[i];
        lit_uniform.model = math::make_translate_matrix(proj->pos);

        gpu_buffer_update(&g->game.lit_uniform, &lit_uniform);
        gpu_mesh_draw(&g->game.cube);
    });

    packed_array_iterate(g->world->particles, [&](uint32_t i) {
        const auto *prt = &g->world->particles[i];
        const auto &templ = c_particle_templates[prt->template_idx];

        const float total_lifetime = remap_to_float(templ.lifetime[0], templ.lifetime[1], prt->rng_seed);
        const float t = prt->lifetime / total_lifetime;

        const v3 size = keyframe_sample(templ.size, t, prt->rng_seed);

        // TODO: Implement rotation
        lit_uniform.model = math::make_translate_matrix(prt->pos);
        lit_uniform.model.m[0][0] = size.x;
        lit_uniform.model.m[1][1] = size.y;
        lit_uniform.model.m[2][2] = size.z;

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
