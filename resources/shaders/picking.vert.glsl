#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;		// ignored but kept for compatibility with existing VAOs
layout(location = 2) in vec2 aTexCoords;	// ignored

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

// no lighting, just pass through
void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
}
