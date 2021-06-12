in vec2 uv;
out vec4 col;

void main() {
    vec4 col_a = vec4(0.55, 0.45, 0.35, 1.0);
    vec4 col_b = vec4(0.45, 0.35, 0.25, 1.0);
    vec2 coord = uv * 200;
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    float grid_line = min(grid.x, grid.y);
    float mixture = 1.0 - min(grid_line, 1.0);
    col = mix(col_a, col_b, mixture);
}
