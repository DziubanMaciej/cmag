#version 330 core
#extension GL_ARB_separate_shader_objects : enable

uniform vec3 color;
uniform vec2 stippleData;

flat layout(location = 1) in vec2 inPrimitiveStartVertexScreenSpace;

out vec4 outFragColor;

void main() {
    if (stippleData[0] > 0) {
        float t = length(inPrimitiveStartVertexScreenSpace.xy - gl_FragCoord.xy);
        t = t / stippleData[0];
        t = fract(t);
        t = step(stippleData[1], t);
        if (t == 1) {
            discard;
        }
    }

    outFragColor = vec4(color, 1.0);
}
