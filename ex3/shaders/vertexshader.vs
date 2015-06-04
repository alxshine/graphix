#version 330


uniform mat4 ProjectionViewMatrix;
uniform mat4 ModelMatrix;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;

uniform vec3 LightPosition;
uniform vec3 CameraPosition;
uniform vec3 LightDirection;

uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;

out vec4 vColor;

void main()
{
	vec3 n = normalize(ProjectionViewMatrix*ModelMatrix*vec4(Normal, 0)).xyz;
	vec3 l = normalize(ProjectionViewMatrix*vec4(LightDirection,0)).xyz;

	float intensity = max(dot(n, l), 0);
	vColor = diffuse*intensity;

	gl_Position = ProjectionViewMatrix*ModelMatrix*vec4(Position,1);
}
