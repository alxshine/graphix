#version 330

uniform mat4 ProjectionViewMatrix;
uniform mat4 ModelMatrix;

layout (location = 0) in vec3 Position;

out vec4 vColor;

void main()
{
   gl_Position = ProjectionViewMatrix*ModelMatrix*vec4(Position.x, Position.y, Position.z, 1.0);
   vColor = vec4(1.0, 1.0, 1.0, 1.0);
}
