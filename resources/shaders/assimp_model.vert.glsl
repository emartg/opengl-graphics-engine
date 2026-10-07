#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

// fragment position, normal and texture coordinates in u_view space to be passed to the fragment shader
out vec3 v_frag_pos;
out vec3 v_normal;
out vec2 v_tex_coords;

// transformation matrices
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

void main()
{
	v_frag_pos		= vec3(u_view * u_model * vec4(aPos, 1.0));
	v_normal		= mat3(transpose(inverse(u_view * u_model))) * aNormal;  
	v_tex_coords	= aTexCoords;

	gl_Position		= u_projection * u_view * u_model * vec4(aPos, 1.0);
}
