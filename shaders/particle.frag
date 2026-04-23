#version 330 core

in vec2 vLocalPos;
out vec4 fragColor;

uniform vec3 uColor;

void main(){
    float dist = length(vLocalPos);
    if(dist > 1.0) discard;
    fragColor = vec4(uColor, 1.0);
}