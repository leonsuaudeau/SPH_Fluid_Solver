#version 330 core

layout (location = 0) in vec2 aPos;

uniform mat4 uTransform;
uniform vec2 uCenter;
uniform float uRadius;

out vec2 vLocalPos;

void main(){
    vec2 worldPos = uCenter + aPos * uRadius;
    vLocalPos = aPos;
    gl_Position = uTransform * vec4(worldPos, 0.0, 1.0);
}