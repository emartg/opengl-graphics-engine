#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

// maximum number of lights in the scene (same as in the fragment shader)
#define MAX_N_DIR_LIGHTS 3
#define MAX_N_POINT_LIGHTS 3
#define MAX_N_SPOTLIGHTS 3

// fragment position, normal and texture coordinates in view space to be passed to the fragment shader
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

// statically sized arrays of light attributes in view space to be passed to the fragment shader
out vec3 DirectionalLightDir[MAX_N_DIR_LIGHTS];
out vec3 PointLightPos[MAX_N_POINT_LIGHTS];
out vec3 SpotlightPos[MAX_N_SPOTLIGHTS];
out vec3 SpotlightDir[MAX_N_SPOTLIGHTS];

// transformation matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// number of lights currently in the scene
uniform int nDirectionalLights;
uniform int nPointLights;
uniform int nSpotlights;

// statically sized arrays of light attributes in world space
uniform vec3 directionalLightDir[MAX_N_DIR_LIGHTS];	
uniform vec3 pointLightPos[MAX_N_POINT_LIGHTS];
uniform vec3 spotlightPos[MAX_N_SPOTLIGHTS];
uniform vec3 spotlightDir[MAX_N_SPOTLIGHTS];

void main()
{
	// loop through all directional lights and transform their attributes from world space to view space
	for (int i = 0; i < nDirectionalLights; i++)
	{
		DirectionalLightDir[i] = vec3(view * vec4(directionalLightDir[i], 0.0));
	}

	// loop through all point lights and transform their attributes from world space to view space
	for (int i = 0; i < nPointLights; i++)
	{
		PointLightPos[i] = vec3(view * vec4(pointLightPos[i], 1.0));
	}

	// loop through all spotlights and transform their attributes from world space to view space
	for (int i = 0; i < nSpotlights; i++)
	{
		SpotlightPos[i] = vec3(view * vec4(spotlightPos[i], 1.0));
		// direction is a 3 component vector, so we don't need to translate it (w = 0.0)
		SpotlightDir[i] = vec3(view * vec4(spotlightDir[i], 0.0));
	}

	FragPos = vec3(view * model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(view * model))) * aNormal;  
	TexCoords = aTexCoords;

	gl_Position = projection * view * model * vec4(aPos, 1.0);
}