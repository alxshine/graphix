#version 330

//texture
uniform sampler2D textureSampler;

uniform sampler2DShadow depth_texture;

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
in vec2 UVcoords;

in vec4 shadow_coord;

out vec4 FragColor;

void main()
{
	//light model coefficients
	float kA = 0.1;
	float kD = 0.5;
	float kS = 0.2;
	float m = 8;

	//normalize all vectors
	vec3 l1 = normalize(vLight1);
	vec3 l2 = normalize(vLight2);
	vec3 n = normalize(vNormal);
	vec3 v = normalize(vView);

	vec3 r1 = normalize((2*n*dot(n, l1)) - l1);
	float iS1 = clamp(kS * pow(dot(r1, v), m), 0, 1);
	vec3 r2 = normalize((2*n*dot(n, l2)) - l2);
	float iS2 = clamp(kS * pow(dot(r2, v), m), 0, 1);

	float iD1 = clamp(kD * dot(n, l1), 0, 1);
	float iD2 = clamp(kD * dot(n, l2), 0, 1);

	vec4 cAmbient = ambient;
	vec4 cDiffuse = diffuse;
	vec4 cSpecular = specular;

	if(cAmbient == vec4(0)){
		cAmbient = texture2D(textureSampler, UVcoords);
		cDiffuse = texture2D(textureSampler, UVcoords);
		cSpecular = vec4(1);
	}

	float fshadow = textureProj(depth_texture, shadow_coord);

	FragColor = showAmbient * kA * cAmbient + fshadow*lI1 * (showDiffuse * iD1 * cDiffuse + showSpecular * iS1 * cSpecular)
		+ lI2 * (showDiffuse * iD2 * cDiffuse + showSpecular * iS2 * cSpecular);
}