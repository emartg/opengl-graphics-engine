#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

// texture coordinates in clip space to be passed to the fragment shader
out vec2 v_tex_coords;

void main()
{
	v_tex_coords	= aTexCoords;

	gl_Position		= vec4(aPos, 1.0);
}
