#version 330

uniform vec4 LightVector;

uniform mat4 ProjectionViewMatrix;
uniform mat4 ModelMatrix;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Color;
layout (location = 2) in vec3 Normal;

out vec4 vColor;

void main()
{
   gl_Position = ProjectionViewMatrix*ModelMatrix*vec4(Position.x, Position.y, Position.z, 1.0);
   vColor = vec4(Color.r, Color.g, Color.b, 1.0);
}
