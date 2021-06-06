#include "dev.h"
#include "globals.h"
#include "imgui/imgui.h"

void dev_loaded()
{
    auto *ctx = (ImGuiContext*)g->plf->dear_imgui_ctx;
    ImGui::SetCurrentContext(ctx);
}

void dev_menu()
{
    ImGui::Begin("Foo");
    ImGui::Button("Foobar!");
    ImGui::End();

    ImGui::Begin("Info");
    ImGui::End();
}

void dev_rotate_cam(m44 &view_mat, const UpdateInfo *upd)
{
    constexpr float zoom_mult = 0.12f;
    constexpr float pan_mult = 0.07f;
    constexpr float rot_mult = 0.0055f;

#if 0
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

    ImGui::Begin("Viewport Cam");
    ImGui::DragFloat4("X", &view_mat.m[0][0], 0.01f);
    ImGui::DragFloat4("Y", &view_mat.m[1][0], 0.01f);
    ImGui::DragFloat4("Z", &view_mat.m[2][0], 0.01f);
    ImGui::DragFloat4("W", &view_mat.m[3][0], 0.01f);
    if(ImGui::Button("Reset")) {
        view_mat = math::m44_identity();
    }
    ImGui::End();
}
