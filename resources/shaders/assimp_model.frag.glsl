#version 420 core
out vec4 FragColor;

// maximum number of lights in the scene (same as in the vertex shader)
#define MAX_N_DIR_LIGHTS 3
#define MAX_N_POINT_LIGHTS 3
#define MAX_N_SPOTLIGHTS 3

// structs to hold light properties
struct DirectionalLight
{
    // lighting components
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
	// lighting components
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	// attenuation factors
	float constant;
	float linear;
	float quadratic;
};

struct Spotlight
{
    // lighting components
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // attenuation factors
    float constant;
    float linear;
    float quadratic;

    // spotlight properties
    float innerCutOff;  // inner cut-off angle of the spotlight cone
    float outerCutOff;  // outer cut-off angle of the spotlight cone
};

// struct to hold material properties
struct Material
{
	sampler2D albedoMap;    // texture sampler for the albedo map (i.e. the diffuse Map)
    sampler2D metallicMap;  // texture sampler for the metallic map (i.e. the specular Map)
	float shininess;
};

// fragment position, normal and texture coordinates in view space passed from the vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// statically sized arrays of light attributes in view space passed from the vertex shader
in vec3 DirectionalLightDir[MAX_N_DIR_LIGHTS];
in vec3 PointLightPos[MAX_N_POINT_LIGHTS];
in vec3 SpotlightPos[MAX_N_SPOTLIGHTS];
in vec3 SpotlightDir[MAX_N_SPOTLIGHTS];

// number of lights currently in the scene
uniform int nDirectionalLights;
uniform int nPointLights;
uniform int nSpotlights;

// statically sized arrays of light structs
uniform DirectionalLight directionalLights[MAX_N_DIR_LIGHTS]; 
uniform PointLight pointLights[MAX_N_POINT_LIGHTS];
uniform Spotlight spotlights[MAX_N_SPOTLIGHTS];

// material properties struct
uniform Material material;

// Calculates the color of a single directional light given the light properties (including the direction),
// the normal, and the view direction (all in view space)
vec3 computeDirectionalLightColor(DirectionalLight light, vec3 directionalLightDir, 
                                  vec3 normal, vec3 viewDir);

// Calculates the color of a single point light given the light properties (including the position),
// the fragment position, the normal, and the view direction (all in view space)
vec3 computePointLightColor(PointLight light, vec3 pointLightPos, 
                            vec3 normal, vec3 fragPos, vec3 viewDir);

// Calculates the color of a single spotlight given the light properties 
// (including the position and direction), the fragment position, the normal,
// and the view direction (all in view space)
vec3 computeSpotlightColor(Spotlight light, vec3 spotlightPos, vec3 spotlightDir, 
                           vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    // light properties
    vec3 normal     = normalize(Normal);
    vec3 viewDir    = normalize(-FragPos);  // since lighting is being calculated in view space, 
                                            // viewPos is (0, 0, 0)

    // initialize fragment color
    vec3 result = vec3(0.0);

    // loop through all directional lights and accumulate their contributions
    for (int i = 0; i < nDirectionalLights; i++)
        result  += computeDirectionalLightColor(directionalLights[i], DirectionalLightDir[i], 
                                                normal, viewDir);

    // loop through all point lights and accumulate their contributions
    for (int i = 0; i < nPointLights; i++)
        result  += computePointLightColor(pointLights[i], PointLightPos[i], 
                                          normal, FragPos, viewDir);

    // loop through all spotlights and accumulate their contributions
    for (int i = 0; i < nSpotlights; i++)
        result  += computeSpotlightColor(spotlights[i], SpotlightPos[i], SpotlightDir[i], 
                                         normal, FragPos, viewDir);

    // set the fragment color
    FragColor   = vec4(result, 1.0);
}

vec3 computeDirectionalLightColor(DirectionalLight light, vec3 directionalLightDir, 
                                  vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-directionalLightDir);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // combine results
    vec3 ambient    = light.ambient * texture(material.albedoMap, TexCoords).rgb;
    vec3 diffuse    = light.diffuse * diff * texture(material.albedoMap, TexCoords).rgb;
    vec3 specular   = light.specular * spec * texture(material.metallicMap, TexCoords).rgb;
    return (ambient + diffuse + specular);
}

vec3 computePointLightColor(PointLight light, vec3 pointLightPos, 
                            vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(pointLightPos - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // attenuation
    float distance      = length(pointLightPos - fragPos);
    float attenuation   = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // combine results
    vec3 ambient    = light.ambient * texture(material.albedoMap, TexCoords).rgb;
    vec3 diffuse    = light.diffuse * diff * texture(material.albedoMap, TexCoords).rgb;
    vec3 specular   = light.specular * spec * texture(material.metallicMap, TexCoords).rgb;
    ambient         *= attenuation;
    diffuse         *= attenuation;
    specular        *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 computeSpotlightColor(Spotlight light, vec3 spotlightPos, vec3 spotlightDir, 
                           vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(spotlightPos - fragPos);

    // diffuse shading
    float diff  = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // attenuation
    float distance      = length(spotlightPos - fragPos);
    float attenuation   = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // spotlight intensity
    float theta     = dot(lightDir, normalize(-spotlightDir));
    float epsilon   = light.innerCutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // combine results
    vec3 ambient    = light.ambient * texture(material.albedoMap, TexCoords).rgb;
    vec3 diffuse    = light.diffuse * diff * texture(material.albedoMap, TexCoords).rgb;
    vec3 specular   = light.specular * spec * texture(material.metallicMap, TexCoords).rgb;
    ambient         *= attenuation * intensity;
    diffuse         *= attenuation * intensity;
    specular        *= attenuation * intensity;
    return (ambient + diffuse + specular);
}