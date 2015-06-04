#version 330


uniform mat4 ProjectionViewMatrix;
uniform mat4 ModelMatrix;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;

uniform vec3 LightPosition;
uniform vec3 CameraPosition;

uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;

out vec4 vColor;

void main()
{
	gl_Position = ProjectionViewMatrix*ModelMatrix*vec4(Position,1);

	//convert normal vector to world space
	vec3 n = normalize(ModelMatrix*vec4(Normal, 0)).xyz;

	//convert position to world space (lightPosition is already in world space)
	vec4 p4 = (ModelMatrix*vec4(Position,1));
	vec3 p = (p4/p4[3]).xyz;

	//calculate vector from vertex to light (in world space)
	vec3 l = normalize(LightPosition - p);

	float intensity = max(dot(n, l), 0);
	vColor = diffuse*intensity;
}
