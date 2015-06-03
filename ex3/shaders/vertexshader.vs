#version 330


uniform mat4 ProjectionViewMatrix;
uniform mat4 ModelMatrix;

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Color;
layout (location = 2) in vec3 Normal;

uniform vec4 LightPosition;
uniform vec4 CameraPosition;

out vec4 vColor;
out vec4 vLight;
out vec4 vNormal;
out vec4 vView;

void main()
{
	gl_Position = ProjectionViewMatrix*ModelMatrix*vec4(Position.x, Position.y, Position.z, 1.0);
	vColor = vec4(Color.r, Color.g, Color.b, 1.0);

	vec4 vPosition = vec4(Position, 1);
	vLight = ProjectionViewMatrix*ModelMatrix*vPosition - ProjectionViewMatrix*LightPosition;
	vNormal = ProjectionViewMatrix*ModelMatrix*(vec4(Normal, 1));
	vView = ProjectionViewMatrix*CameraPosition - ProjectionViewMatrix*ModelMatrix*vec4(Position, 1);
}
