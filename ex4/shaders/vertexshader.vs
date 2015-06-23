#version 330


uniform mat4 ProjectionViewMatrix;
uniform mat4 ModelMatrix;

uniform mat4 shadow_matrix;
//uniform mat4 ViewMatrix;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 UV;

//light positions
uniform vec3 lP1;
uniform vec3 lP2;

uniform vec3 cP;

out vec3 vLight1;
out vec3 vLight2;
out vec3 vNormal;
out vec3 vView;
out vec2 UVcoords;

out vec4 shadow_coord;

void main()
{
	gl_Position = ProjectionViewMatrix*ModelMatrix*vec4(Position,1);

	//convert normal vector to world space
//	vNormal = vec3(normalize(ModelMatrix*vec4(Normal,0)));
	vNormal = Normal;

	//convert position to world space (lP1 is already in world space)
	vec4 p4 = (ModelMatrix*vec4(Position,1));
	vec3 p = vec3(p4);

	//shadow
	shadow_coord = shadow_matrix * p4;

	//calculate vector from vertex to light (in world space)
	vLight1 = normalize(lP1 - p);
	vLight2 = normalize(lP2 - p);

	//view vector
	vView = normalize(cP - p);

	UVcoords = UV;
}
