#version 330

//colors
uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;

//factors for turning the lighting components on and off
uniform float showAmbient;
uniform float showDiffuse;
uniform float showSpecular;

//light intensities
uniform vec4 lI1;
uniform vec4 lI2;

in vec3 vLight1;
in vec3 vLight2;
in vec3 vNormal;
in vec3 vView;

out vec4 FragColor;

void main()
{
	//light model coefficients
	float kA = 0.1;
	float kD = 0.5;
	float kS = 0.2;
	float m = 10;

	//normalize all vectors
	vec3 l1 = normalize(vLight1);
	vec3 l2 = normalize(vLight2);
	vec3 n = normalize(vNormal);
	vec3 v = normalize(vView);

	vec3 r1 = normalize((2*n*dot(n, l1)) - l1);
	float iS1 = max(kS * pow(dot(r1,v), m), 1);
	vec3 r2 = normalize((2*n*dot(n, l2)) - l2);
	float iS2 = max(kS * pow(dot(r2,v), m), 1);

	float iD1 = max(kD * dot(n,l1), 1);
	float iD2 = max(kD * dot(n, l2), 1);

	FragColor = showAmbient * kA * ambient + lI1 * (showDiffuse * kD * iD1 * diffuse + showSpecular * kS * iS1 * specular)
		+ lI2 * (showDiffuse * kD * iD2 * diffuse + showSpecular * kS * iS2 * specular);
}