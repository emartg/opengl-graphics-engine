#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	// position and normal vectors are transformed to view space so that lighting
	// calculations can be performed in view space in the fragment shader
	Position = vec3(view * model * vec4(aPos, 1.0));
	Normal   = mat3(transpose(inverse(view * model))) * aNormal;
	
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}