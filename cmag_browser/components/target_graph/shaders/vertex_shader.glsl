#version 330 core
uniform float depthValue;
uniform mat4 transform;
layout(location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, depthValue, 1.0);
    gl_Position = transform * gl_Position;
}
