#version 330

uniform mat4 VP_matrix;
uniform mat4 ModelMatrix;

layout (location = 0) in vec4 position;

void main(void)
{
    gl_Position=VP_matrix*ModelMatrix* position;
}