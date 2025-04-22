#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

#define MAX_N_POINT_LIGHTS 10	// maximum number of point lights in the scene (same as in the fragment shader)

out vec3 PointLightPos[MAX_N_POINT_LIGHTS];	// statically sized array of point light positions in view space
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform vec3 pointLightPos[MAX_N_POINT_LIGHTS];	// statically sized array of point light positions in world space
uniform int nPointLights;						// actual number of point lights currently in the scene
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	for (int i = 0; i < nPointLights; i++)	// loop through all point lights
		PointLightPos[i] = vec3(view * vec4(pointLightPos[i], 1.0));	// transform from world to view space
	FragPos = vec3(view * model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(view * model))) * aNormal;  
	TexCoords = aTexCoords;

	gl_Position = projection * view * model * vec4(aPos, 1.0);
}