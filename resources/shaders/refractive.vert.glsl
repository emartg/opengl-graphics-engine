#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 v_position;
out vec3 v_normal;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

void main()
{
	// position and normal vectors are transformed to view space so that lighting
	// calculations can be performed in view space in the fragment shader
	v_position	= vec3(u_view * u_model * vec4(aPos, 1.0));
	v_normal	= mat3(transpose(inverse(u_view * u_model))) * aNormal;
	
	gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
}