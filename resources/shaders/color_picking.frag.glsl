#version 420 core

// Output color
out vec4 FragColor;

// Uniform for the object's unique color ID
uniform vec3 objectColor;

void main()
{
	// Output the unique color for this object
	FragColor = vec4(objectColor, 1.0);
}