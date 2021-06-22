#include "struct_defs.h"
#define M_PI 3.1415926535897932384626433832795

in vec2 uv;
in vec3 ray_dir;

out vec4 col;

layout (std140) uniform UniformBuffer {
    SkyUniform u;
};

void main() {
    float x = (atan(ray_dir.x, ray_dir.z) / M_PI) * 0.5 + 0.5;
    float y = pow(ray_dir.y, 0.4);
    col = vec4(x, y, 0.0, 1.0);
}
