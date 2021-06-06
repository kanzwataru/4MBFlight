#pragma once
#if WITH_DEV
#include "mathlib.h"
#include "platform/platform.h"

void dev_loaded();
void dev_menu();

void dev_rotate_cam(m44 &view_mat, const UpdateInfo *upd);

#endif
