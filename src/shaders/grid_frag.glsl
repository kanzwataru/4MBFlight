in float depth;
in vec2 uv;
out vec4 col;

void main() {
    vec2 coord = uv * 300;
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    float grid_line = min(grid.x, grid.y);
    float mixture = 1.0 - min(grid_line, 1.0);

    float fade = 1.0 - clamp(depth * 0.0025, 0.0, 1.0);

    col = vec4(1.0, 1.0, 1.0, mixture * fade);
}
