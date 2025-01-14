#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 LightPos;		// light position in view space
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform vec3 lightPos;	// light position in world space
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	LightPos = vec3(view * vec4(lightPos, 1.0));
	FragPos = vec3(view * model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(view * model))) * aNormal;  
	TexCoords = aTexCoords;

	gl_Position = projection * view * model * vec4(aPos, 1.0);
}