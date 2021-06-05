#include "struct_defs.h"
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_col;

out vec4 vert_col;

layout (std140) uniform UniformBuffer {
    VertColUniform u;
};

void main() {
    gl_Position = u.proj * u.view * u.model * vec4(in_position, 1.0);
    vert_col = in_col;
}

