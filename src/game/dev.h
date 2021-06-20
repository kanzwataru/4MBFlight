#pragma once
#if WITH_DEV
#include "mathlib.h"
#include "platform/platform.h"
#include "imgui/imgui.h"

#define DEV_MODULE_STATE_SIZE	128

enum class EditorMode : int{
    Ingame,
    EditParticles
};

void dev_init();
void dev_loaded();
void dev_update(const UpdateInfo *upd, PlatformOptions *options);

void dev_rotate_cam(m44 &view_mat, const UpdateInfo *upd);

#endif
