#version 420 core
out vec4 FragColor;

struct Light
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

struct Material
{
	sampler2D diffuse1;
	sampler2D specular1;
	float shininess;
};

in vec3 LightPos;	// light position already in view space
in vec3 FragPos;	// fragment position already in view space
in vec3 Normal;		// normal already in view space
in vec2 TexCoords;

uniform Light light;
uniform Material material;

void main()
{
	// ambient component
	vec3 ambient = light.ambient * texture(material.diffuse1, TexCoords).rgb;

	// diffuse component
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(LightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse1, TexCoords).rgb;

	// specular component
	vec3 viewDir = normalize(-FragPos);		// we already are in view space so view position is (0, 0, 0)
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular1, TexCoords).rgb;

	// attenuation
	float distance = length(LightPos - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	
	// combine results
	vec3 result = (ambient + diffuse + specular) * attenuation;
	FragColor = vec4(result, 1.0);
}