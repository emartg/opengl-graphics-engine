#version 450 core
out vec4 FragColor;

uniform vec3 u_albedo;

void main()
{
	FragColor = vec4(u_albedo, 1.0);
}