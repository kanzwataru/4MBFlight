#include "struct_defs.h"
in vec2 uv;

out vec4 col;

layout (std140) uniform UniformBuffer {
    SkyUniform u;
};

void main() {
    float v = uv.y - sin(u.cam_rot.x);

    col = vec4(0.0, v, 0.0, 1.0);
}
