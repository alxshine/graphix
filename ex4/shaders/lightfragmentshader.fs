#version 330
layout (location = 0) out vec4 color;
void main(void)
{
    gl_FragDepth = gl_FragCoord.z;
}