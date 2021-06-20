#include "struct_defs.h"
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;

out vec2 uv;

layout (std140) uniform UniformBuffer {
    SkyUniform u;
};

void main() {
    gl_Position = vec4(in_position, 1.0);
    uv = in_uv;
}
