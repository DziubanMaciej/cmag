#version 330 core
out vec4 FragColor;
uniform vec3 color;
uniform ivec2 stippleData;
void main() {
    int summedCoords = int(gl_FragCoord.x) + int(gl_FragCoord.y);
    if (summedCoords % stippleData[0] > stippleData[1]) {
        discard;
    }

    FragColor = vec4(color, 1.0);
}
