#version 330


uniform mat4 ProjectionViewMatrix;
uniform mat4 ModelMatrix;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;

//light positions
uniform vec3 lP1;
uniform vec3 lP2;

uniform vec3 cP;

out vec3 vLight1;
out vec3 vLight2;
out vec3 vNormal;
out vec3 vView;

void main()
{
	gl_Position = ProjectionViewMatrix*ModelMatrix*vec4(Position,1);

	//convert normal vector to world space
//	vNormal = vec3(normalize(ModelMatrix*vec4(Normal,0)));
	vNormal = Normal;

	//convert position to world space (lP1 is already in world space)
	vec4 p4 = (ModelMatrix*vec4(Position,1));
	vec3 p = (p4/p4.w).xyz;

	//calculate vector from vertex to light (in world space)
	vLight1 = normalize(lP1 - p);
	vLight2 = normalize(lP2 - p);

	//view vector
	vView = normalize(cP - p);
}
