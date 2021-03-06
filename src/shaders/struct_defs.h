#pragma once
#include "shader_multilang.h"

struct VertColUniform {
	mat4 model;
	mat4 view;
	mat4 proj;
};

struct LitUniform {
    mat4 model;
    mat4 view;
    mat4 proj;

    vec3 light_dir;
};

struct SkyUniform {
    mat4 view;
    mat4 proj;
    float width;
    float height;
    float fov; // radians
};
