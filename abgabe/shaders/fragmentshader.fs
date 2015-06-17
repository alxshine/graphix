#version 330

in vec4 vColor;
//in vec4 vLight;
//in vec4 vNormal;
//in vec4 vView;

out vec4 FragColor;

void main()
{
//	vec4 n = normalize(vNormal);
//	vec4 v = normalize(vView);
//	vec4 l = normalize(vLight);
//
//    float kA = 0.2;
//    float kD = 0.7;
//    float kS = 0.1;
//    float m = 0.5;
//
//    vec4 r = (2*v*dot(v, l)) - l;
//
//    FragColor = vColor*(kA + max(kD * dot(n, l),0) + max(kS * pow(dot(r, v), m),0));
	FragColor = vColor;
}