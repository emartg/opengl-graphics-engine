#version 420 core
layout(location = 0) in vec3 aPos;

out vec3 v_tex_coords;

uniform mat4 u_projection;
uniform mat4 u_view;

void main()
{
	v_tex_coords	= aPos; // pass through the position as texture coordinates

	// push depth to far plane so that the cube is rendered behind all other geometry
	vec4 pos		= u_projection * u_view * vec4(aPos, 1.0);
	gl_Position		= pos.xyww; // set w component to the z component
}