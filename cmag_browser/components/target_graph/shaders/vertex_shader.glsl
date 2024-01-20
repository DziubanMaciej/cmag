#version 330 core
#extension GL_ARB_separate_shader_objects : enable

uniform float depthValue;
uniform mat4 transform;
uniform vec2 screenSize;

in vec2 inPosition;

flat layout(location = 1) out vec2 outPrimitiveStartVertexScreenSpace;

void main() {
    gl_Position = vec4(inPosition, depthValue, 1.0);
    gl_Position = transform * gl_Position;

    outPrimitiveStartVertexScreenSpace = gl_Position.xy / gl_Position.w; // clip space <-1, 1>
    outPrimitiveStartVertexScreenSpace = (outPrimitiveStartVertexScreenSpace + 1) / 2; // <0, 1>
    outPrimitiveStartVertexScreenSpace = outPrimitiveStartVertexScreenSpace * screenSize; // <0, screenSize>
}
