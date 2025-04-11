#version 420 core
out vec4 FragColor;

in vec3 FragPos;	// fragment position already in view space
in vec3 Normal;		// normal already in view space
in vec2 TexCoords;

uniform vec3 albedo;

void main()
{
	FragColor = vec4(albedo, 1.0);
}