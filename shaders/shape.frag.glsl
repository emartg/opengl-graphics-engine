#version 420 core
out vec4 FragColor;

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
    float cutOff;       // inner cut-off angle of the spotlight cone
    float outerCutOff;  // outer cut-off angle of the spotlight cone
};

struct Material
{
	vec3 albedo;
	float shininess;
};

#define MAX_N_POINT_LIGHTS 5 // maximum number of point lights in the scene (same as in the vertex shader)
#define MAX_N_SPOTLIGHTS 5   // maximum number of spotlights in the scene (same as in the vertex shader)

in vec3 PointLightPos[MAX_N_POINT_LIGHTS];  // statically sized array of point light positions in view space
in vec3 SpotlightPos[MAX_N_SPOTLIGHTS];     // statically sized array of spotlight positions in view space
in vec3 SpotlightDir[MAX_N_SPOTLIGHTS];     // statically sized array of spotlight directions in view space
in vec3 FragPos;                            // fragment position already in view space
in vec3 Normal;                             // normal already in view space
in vec2 TexCoords;

uniform PointLight pointLights[MAX_N_POINT_LIGHTS]; // statically sized array of point light structs
uniform Spotlight spotlights[MAX_N_SPOTLIGHTS];     // statically sized array of spotlight structs
uniform int nPointLights;			                // actual number of point lights currently in the scene
uniform int nSpotlights;			                // actual number of spotlights currently in the scene
uniform Material material;

// calculates the color of a single point light given the light properties (including the position),
// the fragment position, the normal and the view direction (all in view space)
vec3 computePointLightColor(PointLight light, vec3 pointLightPos, vec3 normal, vec3 fragPos, vec3 viewDir);

// canculates the color of a single spotlight given the light properties (including the position and direction),
// the fragment position, the normal and the view direction (all in view space)
vec3 computeSpotlightColor(Spotlight light, vec3 spotlightPos, vec3 spotlightDir, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    // light properties
    vec3 normal     = normalize(Normal);
    vec3 viewDir    = normalize(-FragPos);  // since lighting is being calculated in view space, viewPos is (0, 0, 0)

    // initialize fragment color
    vec3 result = vec3(0.0);

    // loop through all point lights
    for (int i = 0; i < nPointLights; i++)
        result  += computePointLightColor(pointLights[i], PointLightPos[i], normal, FragPos, viewDir);

    // loop through all spotlights
    for (int i = 0; i < nSpotlights; i++)
        result  += computeSpotlightColor(spotlights[i], SpotlightPos[i], SpotlightDir[i], normal, FragPos, viewDir);

    // set the fragment color
    FragColor   = vec4(result, 1.0);
}

vec3 computePointLightColor(PointLight light, vec3 pointLightPos, vec3 normal, vec3 fragPos, vec3 viewDir)
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
    vec3 ambient    = light.ambient * material.albedo;
    vec3 diffuse    = light.diffuse * diff * material.albedo;
    vec3 specular   = light.specular * spec;    // the object is completely shiny since there is no specular map
    ambient         *= attenuation;
    diffuse         *= attenuation;
    specular        *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 computeSpotlightColor(Spotlight light, vec3 spotlightPos, vec3 spotlightDir, vec3 normal, vec3 fragPos, vec3 viewDir)
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
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // combine results
    vec3 ambient    = light.ambient * material.albedo;
    vec3 diffuse    = light.diffuse * diff * material.albedo;
    vec3 specular   = light.specular * spec;    // the object is completely shiny since there is no specular map
    ambient         *= attenuation * intensity;
    diffuse         *= attenuation * intensity;
    specular        *= attenuation * intensity;
    return (ambient + diffuse + specular);
}