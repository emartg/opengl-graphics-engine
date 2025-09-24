#version 420 core
layout(location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	TexCoords = aPos;

	// push depth to far plane so that the cube is rendered behind all other geometry
	vec4 pos = projection * view * vec4(aPos, 1.0);
	gl_Position = pos.xyww; // set w component to the z component
}