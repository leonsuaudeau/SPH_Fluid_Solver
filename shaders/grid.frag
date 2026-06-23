#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform mat4 uInvTransform;
uniform vec2 uOrigin;
uniform vec2 uSize;

void main() {
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 world = uInvTransform * vec4(ndc, 0.0, 1.0);
    vec2 worldPos = world.xy / world.w;

    vec2 grid = worldPos - uOrigin;

    vec2 minCorner = vec2(0.0);
    vec2 maxCorner = uSize;

    bool inside = grid.x >= minCorner.x && grid.y >= minCorner.y && grid.x <= maxCorner.x && grid.y <= maxCorner.y;

    vec2 edgeDist = min(grid - minCorner, maxCorner - grid);
    float dist = min(edgeDist.x, edgeDist.y);

    float lineWidth = 0.2;
    float outline = 1.0 - smoothstep(lineWidth, lineWidth + fwidth(dist), dist);

    outline *= inside? 1.0 : 0.0;

    vec3 baseColor = inside? vec3(0.15, 0.15, 0.18) : vec3(0.02, 0.02, 0.025);
    vec3 color = mix(baseColor, vec3(1.0), outline);
    FragColor = vec4(color, 1.0);
}
