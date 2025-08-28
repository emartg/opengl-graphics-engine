#version 420 core

// Vertex attributes
layout (location = 0) in vec3 aPos;

// Uniform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	// transform vertex position to clip space
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}