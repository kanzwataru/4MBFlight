#include "struct_defs.h"
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;

out float depth;
out vec2 uv;

layout (std140) uniform UniformBuffer {
    VertColUniform u;
};

void main() {
    vec4 pos = u.proj * u.view * u.model * vec4(in_position, 1.0);
    gl_Position = pos;
    uv = in_uv;
    depth = pos.z;
}
