#version 450 core
layout (location = 0) in vec3 aPos;

out vec3 v_local_pos;

uniform mat4 u_projection;
uniform mat4 u_view;

void main()
{
	v_local_pos = aPos;
	gl_Position = u_projection * u_view * vec4(aPos, 1.0);
}