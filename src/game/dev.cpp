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
}
