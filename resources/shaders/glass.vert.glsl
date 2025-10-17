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
	// normal and position vectors are tronsformed to world space here, so that the environment
	// mapping in the fragment shader can be done in said space
	Normal = mat3(transpose(inverse(model))) * aNormal;	// normal vectors need to be transformed by 
														// the inverse transpose of the model matrix
	Position = vec3(model * vec4(aPos, 1.0));
	
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}