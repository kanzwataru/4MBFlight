in vec3 norm;
in vec2 uv;

out vec4 col;

void main() {
    col = vec4(uv.x, uv.y, 0.0, 1.0);
}
