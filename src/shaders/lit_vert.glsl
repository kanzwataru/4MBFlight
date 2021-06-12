#include "struct_defs.h"
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec2 in_uv;

out vec3 norm;
out vec2 uv;

layout (std140) uniform UniformBuffer {
    LitUniform u;
};

void main() {
    gl_Position = u.proj * u.view * u.model * vec4(in_position, 1.0);
    uv = in_uv;
    norm = mat3(transpose(inverse(u.model))) * in_norm;
}
