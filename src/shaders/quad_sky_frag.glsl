#include "struct_defs.h"
in vec2 uv;
in vec3 ray_dir;

out vec4 col;

layout (std140) uniform UniformBuffer {
    SkyUniform u;
};

void main() {
    //float v = uv.y - sin(u.cam_rot.x);

    //col = vec4(0.0, v, 0.0, 1.0);
    //col = vec4(0.0, uv.y, 0.0, 1.0);
    float x = sin(uv.x * 50.0) * 0.5 + 0.5;
    //float y = sin(uv.y * 100.0) * 0.5 + 0.5;
    //float y = abs(uv.y);
    float y = uv.y > 0.0 ? 1.0 : 0.0;
    //col = vec4(x, y, 0.0, 1.0);
    col = vec4(ray_dir, 1.0);
    //col = vec4(uv.x, uv.y, 0.0, 1.0);
}
