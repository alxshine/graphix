#version 330


uniform mat4 ProjectionViewMatrix;
uniform mat4 ModelMatrix;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;

uniform vec3 lP1;
uniform vec4 lI1;

uniform vec3 lP2;
uniform vec4 lI2;

uniform vec3 cP;

uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;

//factors for turning the lighting components on and off
uniform float showAmbient;
uniform float showDiffuse;
uniform float showSpecular;

out vec4 vColor;

void main()
{
	gl_Position = ProjectionViewMatrix*ModelMatrix*vec4(Position,1);

	//convert normal vector to world space
	vec3 n = normalize(Normal);

	//convert position to world space (lP1 is already in world space)
	vec4 p4 = (ModelMatrix*vec4(Position,1));
	vec3 p = (p4/p4.w).xyz;

	//calculate vector from vertex to light (in world space)
	vec3 l1 = normalize(lP1 - p);
	vec3 l2 = normalize(lP2 - p);

	//diffuse reflection
	float iDiffuse1 = showDiffuse*max(dot(n, l1), 0);
	float iDiffuse2 = showDiffuse*max(dot(n, l2), 0);

	//prepare vectors for specular reflection
	vec3 r1 = normalize((2*n*dot(n,l1))-l1);
	vec3 r2 = normalize((2*n*dot(n,l2))-l2);

	vec3 v = normalize(cP - p);
	float m = 10;

	float iSpecular1 = showSpecular*max(pow(dot(r1, v), m), 0);
	float iSpecular2 = showSpecular*max(pow(dot(r2, v), m), 0);

	//light model coefficients
	float kA = 0.1;
	float kD = 0.5;
	float kS = 0.2;

	//full color calculation
	vColor = showAmbient*kA*ambient + lI1 * (kD*iDiffuse1*diffuse + kS*iSpecular1*specular) + lI2 * (kD*iDiffuse2*diffuse + kS*iSpecular2*specular);
}
