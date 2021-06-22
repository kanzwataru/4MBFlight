#include "struct_defs.h"
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;

out vec2 uv;
out vec3 ray_dir;

layout (std140) uniform UniformBuffer {
    SkyUniform u;
};

void main() {
    vec2 px = in_uv * vec2(u.width, u.height);
    float aspect = u.width / u.height;
    float x =  (2.0 * (px.x + 0.5) / u.width - 1)  * tan(u.fov / 2.0) * aspect;
    float y = -(2.0 * (px.y + 0.5) / u.height - 1) * tan(u.fov / 2.0);

    vec4 ray_dir_local = normalize(vec4(x, -y, -1.0, 0.0));
    vec4 ray_dir_world = normalize(ray_dir_local * u.view);

    gl_Position = vec4(in_position, 1.0);
    uv = ray_dir.xy;
    ray_dir = ray_dir_world.xyz;
}
