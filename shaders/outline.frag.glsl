#version 420 core
out vec4 FragColor;

uniform vec3 outlineAlbedo;

void main()
{
	FragColor = vec4(outlineAlbedo, 1.0);
}