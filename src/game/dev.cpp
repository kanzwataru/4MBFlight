#include "dev.h"
#include "game.h"
#include "imgui/imgui.h"

struct DeveloperGameState {
    float fps_cam_speed = 2.0f;
    bool stay_ejected = false;
    EditorMode mode;
};
static_assert (sizeof(DeveloperGameState) <= DEV_MODULE_STATE_SIZE, "");

static DeveloperGameState *g_dev;

void dev_loaded()
{
    auto *ctx = (ImGuiContext*)g->plf->dear_imgui_ctx;
    ImGui::SetCurrentContext(ctx);

    g_dev = (DeveloperGameState *)g->dev_module;
}

void dev_init()
{
    *g_dev = DeveloperGameState();
}

void dev_menu(const UpdateInfo *upd, PlatformOptions *options)
{
    bool toggle_game_paused = false;
    if(upd->devinput.f2_key && upd->devinput_prev.f2_key != upd->devinput.f2_key) {
        toggle_game_paused = true;
    }

    ImGui::Begin("Toolbar");
    //ImGui::BeginMainMenuBar();
    if(g->game.paused) {
        if(ImGui::Button(">", {32, 20}) || toggle_game_paused) {
            g->game.paused = false;
            g->game.ejected = g_dev->stay_ejected;
            options->reset_axes_next_frame = true;
        }
    }
    else {
        if(ImGui::Button("||", {32, 20}) || toggle_game_paused) {
            g->game.paused = true;
            g->game.ejected = true;
        }
    }
    ImGui::SameLine();
    ImGui::Checkbox("Stay Ejected", &g_dev->stay_ejected);
    ImGui::SameLine();

    const char *modes_list[] = {
        "Game",
        "Particle Editor"
    };

    ImGui::Combo("Mode", (int*)&g_dev->mode, modes_list, countof(modes_list));
    //ImGui::EndMainMenuBar();
    ImGui::End();

    ImGui::Begin("Info");
    if(ImGui::CollapsingHeader("Input")) {
        ImGui::LabelText("Fire Key", "Last Was Down: %d Down: %d", upd->input.fire.last_down, upd->input.fire.down);
        ImGui::LabelText("Pitch Axis", "Value: %f Delta: %f", upd->input.pitch.value, upd->input.pitch.delta);
        ImGui::LabelText("Roll Axis", "Value: %f Delta: %f", upd->input.roll.value, upd->input.roll.delta);
        ImGui::LabelText("Yaw Axis", "Value: %f Delta: %f", upd->input.yaw.value, upd->input.yaw.delta);
        ImGui::LabelText("Throttle Axis", "Value: %f Delta: %f", upd->input.throttle.value, upd->input.throttle.delta);

        ImGui::Separator();
        ImGui::LabelText("Dev Mouse X", "Value: %f", upd->devinput.mouse_rel[0]);
        ImGui::LabelText("Dev Mouse Btn[0]", "Value: %d", upd->devinput.mouse_button[0]);
    }
    ImGui::Separator();
    ImGui::End();

    ImGui::Begin("Systems");
    ImGui::LabelText("Particle Count", "Count: %d", packed_array_count(g->world->particles));
    ImGui::Separator();
    if(ImGui::CollapsingHeader("ParticleEffects")) {
        uint32_t count = packed_array_count(g->world->particle_effects);
        ImGui::LabelText("Particle Effect Count", "%d", count);
        ImGui::Separator();

        packed_array_iterate(g->world->particle_effects, [&](uint32_t i) {
            auto *eff = &g->world->particle_effects[i];
            ImGui::PushID(i);
            ImGui::LabelText("Index", "[%d]", i);
            for(auto &emitter : eff->emitters) {
                if(emitter.template_idx == 0)
                    break;

                ImGui::SliderFloat("Lifetime", &emitter.lifetime, 0.0, 0.0);
                ImGui::SliderFloat("Spawn Counter", &emitter.spawn_counter, 0.0, 0.0);
            }
            //ImGui::SliderFloat()
            ImGui::PopID();
        });
    }
    if(ImGui::CollapsingHeader("Projectiles", ImGuiTreeNodeFlags_DefaultOpen)) {
        uint32_t count = packed_array_count(g->world->projectiles);
        ImGui::LabelText("Projectile Count", "%d", count);
        ImGui::Separator();

        packed_array_iterate(g->world->projectiles, [&](uint32_t i) {
            auto *proj = &g->world->projectiles[i];
            ImGui::PushID(i);
            ImGui::LabelText("Index", "[%d]", i);
            ImGui::SliderFloat3("Pos", &proj->pos.x, 0.0f, 0.0f);
            ImGui::SliderFloat3("Vel", &proj->vel.x, 0.0f, 0.0f);
            ImGui::PopID();
        });
    }
    ImGui::End();

    //ImGui::ShowDemoWindow();
}

static void dev_interact_firstperson_cam(m44 &view_mat, const UpdateInfo *upd)
{
    const float move_speed = 0.2f * g_dev->fps_cam_speed;
    const float rot_mult = 0.0055f;

    /* rotation */
    if(upd->devinput.mouse_button[2]) {
        float horiz = upd->devinput.mouse_rel[0] * rot_mult;
        float vert = upd->devinput.mouse_rel[1] * rot_mult;

        m44 vert_rot_mat = math::make_rot_matrix({1.0f, 0.0f, 0.0f}, vert);

        view_mat = vert_rot_mat * view_mat;
        m44 horiz_rot_mat = math::make_rot_matrix({view_mat.m[1][0], view_mat.m[1][1], view_mat.m[1][2]}, horiz);
        view_mat = horiz_rot_mat * view_mat;
    }

    v3 left = {1.0f, 0.0f, 0.0f};
    v3 forward = {0.0f, 0.0f, 1.0f};
    v3 pos = math::v3_from_axis(view_mat, 3);

    /* movement */
    if(upd->devinput.w) {
        pos += forward * move_speed;
    }
    if(upd->devinput.s) {
        pos -= forward * move_speed;
    }
    if(upd->devinput.a) {
        pos += left * move_speed;
    }
    if(upd->devinput.d) {
        pos -= left * move_speed;
    }

    view_mat.m[3][0] = pos.x;
    view_mat.m[3][1] = pos.y;
    view_mat.m[3][2] = pos.z;
}

static void dev_interact_tumble_cam(m44 &view_mat, const UpdateInfo *upd)
{
    constexpr float zoom_mult = 0.12f;
    constexpr float pan_mult = 0.025f;
    constexpr float rot_mult = 0.0045f;

#if 1
    // Single-button controls
    bool is_zoom = upd->devinput.shift_key && upd->devinput.alt_key && upd->devinput.mouse_button[0];
    bool is_pan = upd->devinput.shift_key && upd->devinput.mouse_button[0];
    bool is_rot = upd->devinput.alt_key && upd->devinput.mouse_button[0];
#else
    // Maya controls
    bool is_zoom = upd->devinput.alt_key && upd->devinput.mouse_button[2];
    bool is_pan = upd->devinput.alt_key && upd->devinput.mouse_button[1];
    bool is_rot = upd->devinput.alt_key && upd->devinput.mouse_button[0];
#endif

    if(is_zoom) {
        float delta = (upd->devinput.mouse_rel[0] + upd->devinput.mouse_rel[1]);
        view_mat.m[3][2] += delta * zoom_mult;
    }
    else if(is_pan) {
        view_mat.m[3][0] += upd->devinput.mouse_rel[0] * pan_mult;
        view_mat.m[3][1] -= upd->devinput.mouse_rel[1] * pan_mult;
    }
    else if(is_rot) {
        /* rotate */
        float horiz = upd->devinput.mouse_rel[0] * rot_mult;
        float vert = upd->devinput.mouse_rel[1] * rot_mult;

        v3 hz = {sin(horiz), 0.0f, cos(horiz)};
        v3 hy = {0.0f, 1.0f, 0.0f};
        v3 hx = math::normal(math::cross(hy, hz));
        hy = math::normal(math::cross(hz, hx));

        m44 horiz_rot_mat = {{
            {hx.x, hx.y, hx.z, 0.0f},
            {hy.x, hy.y, hy.z, 0.0f},
            {hz.x, hz.y, hz.z, 0.0f},
            {0, 0, 0, 1}
        }};

        v3 vy = {0.0f, cos(vert), sin(vert)};
        v3 vx = {1.0f, 0.0f, 0.0f};
        v3 vz = math::normal(math::cross(vx, vy));
        vx = math::normal(math::cross(vy, vz));

        m44 vert_rot_mat = {{
            {vx.x, vx.y, vx.z, 0.0f},
            {vy.x, vy.y, vy.z, 0.0f},
            {vz.x, vz.y, vz.z, 0.0f},
            {0, 0, 0, 1}
        }};

        vert_rot_mat.m[3][0] = view_mat.m[3][0];
        vert_rot_mat.m[3][1] = view_mat.m[3][1];
        vert_rot_mat.m[3][2] = view_mat.m[3][2];
        vert_rot_mat.m[3][3] = view_mat.m[3][3];
        view_mat.m[3][0] = 0.0f;
        view_mat.m[3][1] = 0.0f;
        view_mat.m[3][2] = 0.0f;
        view_mat.m[3][3] = 1.0f;
        view_mat = vert_rot_mat * view_mat * horiz_rot_mat;
    }
}

void dev_rotate_cam(m44 &view_mat, const UpdateInfo *upd)
{
    if(!upd->devinput.mouse_button[0] && !upd->devinput.alt_key) {
        dev_interact_firstperson_cam(view_mat, upd);
    }
    else {
        dev_interact_tumble_cam(view_mat, upd);
    }

    ImGui::Begin("Viewport Cam");
    ImGui::DragFloat4("X", &view_mat.m[0][0], 0.01f);
    ImGui::DragFloat4("Y", &view_mat.m[1][0], 0.01f);
    ImGui::DragFloat4("Z", &view_mat.m[2][0], 0.01f);
    ImGui::DragFloat4("W", &view_mat.m[3][0], 0.01f);
    ImGui::SliderFloat("FPS Cam Speed", &g_dev->fps_cam_speed, 1.0f, 15.0f);
    if(ImGui::Button("Reset")) {
        view_mat = math::m44_identity();
    }
    ImGui::End();
}
