#version 450 core
out vec4 FragColor;

// maximum number of lights in the scene (same as in the vertex shader)
#define MAX_DIR_LIGHTS_COUNT	3
#define MAX_POINT_LIGHTS_COUNT	3
#define MAX_SPOTLIGHTS_COUNT	3

// structs to hold light properties
struct Directional_Light
{
	// lighting components
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Point_Light
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
	float inner_cutoff;  // inner cut-off angle of the spotlight cone
	float outer_cutoff;  // outer cut-off angle of the spotlight cone
};

// struct to hold material properties
struct Material
{
	sampler2D albedo_map;	// texture sampler for the albedo map (i.e. the diffuse map)
	sampler2D metallic_map;	// texture sampler for the metallic map (i.e. the specular map)
	sampler2D opacity_map;	// texture sampler for the opacity map (i.e. the alpha map)

	int has_albedo_map; 	// flag indicating whether an albedo map is used (0 or 1)
	int has_metallic_map;	// flag indicating whether a metallic map is used (0 or 1)
	int has_opacity_map;	// flag indicating whether an opacity map is used (0 or 1)

	float shininess;
	float base_opacity;		// model's base opacity (used if no opacity map is provided)
};

// fragment position, normal and texture coordinates in view space passed from the vertex shader
in vec3 v_frag_pos;
in vec3 v_normal;
in vec2 v_tex_coords;

// statically sized arrays of light attributes in view space passed from the vertex shader
in vec3 v_directional_light_dir[MAX_DIR_LIGHTS_COUNT];
in vec3 v_point_light_pos[MAX_POINT_LIGHTS_COUNT];
in vec3 v_spotlight_pos[MAX_SPOTLIGHTS_COUNT];
in vec3 v_spotlight_dir[MAX_SPOTLIGHTS_COUNT];

// number of lights currently in the scene
uniform int u_directional_light_count;
uniform int u_point_light_count;
uniform int u_spotlight_count;

// statically sized arrays of light structs
uniform Directional_Light u_directional_lights[MAX_DIR_LIGHTS_COUNT]; 
uniform Point_Light u_point_lights[MAX_POINT_LIGHTS_COUNT];
uniform Spotlight u_spotlights[MAX_SPOTLIGHTS_COUNT];

// material properties struct
uniform Material u_material;

// Helper functions to fetch with fallback if no texture is used
vec3 get_albedo_component();
vec3 get_metallic_component();
float get_opacity_component();

// Calculates the color of a single directional light given the light properties (including the direction),
// the normal, and the view direction (all in view space)
vec3 compute_directional_light_component(Directional_Light light, vec3 directional_light_dir, 
										 vec3 normal, vec3 view_dir);

// Calculates the color of a single point light given the light properties (including the position),
// the fragment position, the normal, and the view direction (all in view space)
vec3 compute_point_light_component(Point_Light light, vec3 point_light_pos, 
								   vec3 normal, vec3 frag_pos, vec3 view_dir);

// Calculates the color of a single spotlight given the light properties 
// (including the position and direction), the fragment position, the normal,
// and the view direction (all in view space)
vec3 compute_spotlight_component(Spotlight light, vec3 spotlight_pos, vec3 spotlight_dir, 
								 vec3 normal, vec3 frag_pos, vec3 view_dir);

void main()
{
	// light properties
	vec3 normal   = normalize(v_normal);
	vec3 view_dir = normalize(-v_frag_pos);	// since lighting is being calculated in view space,
											// viewPos is (0, 0, 0)

	// initialize fragment color
	vec3 result = vec3(0.0);

	// loop through all directional lights and accumulate their contributions
	for (int i = 0; i < u_directional_light_count; i++)
		result  += compute_directional_light_component(u_directional_lights[i], v_directional_light_dir[i],
													   normal, view_dir);

	// loop through all point lights and accumulate their contributions
	for (int i = 0; i < u_point_light_count; i++)
		result  += compute_point_light_component(u_point_lights[i], v_point_light_pos[i], 
												 normal, v_frag_pos, view_dir);

	// loop through all spotlights and accumulate their contributions
	for (int i = 0; i < u_spotlight_count; i++)
		result  += compute_spotlight_component(u_spotlights[i], v_spotlight_pos[i], v_spotlight_dir[i], 
											   normal, v_frag_pos, view_dir);

	// get final opacity either from the opacity map (if it exists) or from the base opacity uniform
	float opacity = get_opacity_component();

	// discard nearly transparent fragments to improve performance and avoid blending issues
	if (opacity < 0.1) discard;

	// set the fragment color with the computed lighting result and the final opacity
	FragColor = vec4(result, opacity);
}

vec3 get_albedo_component()
{
	vec3 sampled_color = texture(u_material.albedo_map, v_tex_coords).rgb;
	// fallback to white so lighting remains visible if no albedo map is used
	return mix(vec3(1.0), sampled_color, float(u_material.has_albedo_map));
}

vec3 get_metallic_component()
{
	vec3 sampled_color = texture(u_material.metallic_map, v_tex_coords).rgb;
	// fallback to white so specular highlights remain visible if no metallic map is used
	return mix(vec3(1.0), sampled_color, float(u_material.has_metallic_map));
}

float get_opacity_component()
{
	float opacity = u_material.base_opacity; // default opacity from the base opacity uniform

	// if an opacity map is used, fetch the opacity from the red channel of the texture
	// (convention for opacity maps is to store opacity in the red channel) 
	// and combine it with the base opacity for overall control
	if (u_material.has_opacity_map == 1)
	{
		// sample the red channel of the opacity map to get the opacity value
		opacity = texture(u_material.opacity_map, v_tex_coords).r;
		// combine the sampled opacity with the base opacity to get the final opacity value for the fragment
		// (common convention is to multiply the base opacity by the sampled opacity from the opacity map)
		opacity *= u_material.base_opacity;
	}
	// if no opacity map is used but an albedo map is used,
	// the alpha channel of the albedo map can be used for opacity (if it exists in the texture);
	// and in any case, the alpha channel is combined with the base opacity for overall control
	else if (u_material.has_albedo_map == 1)
	{
		// sample the alpha channel of the albedo map
		// (if the texture doesn't have an alpha channel, this returns 1.0)
		float albedo_alpha = texture(u_material.albedo_map, v_tex_coords).a;
		// combine the albedo alpha with the base opacity to get the final opacity value for the fragment
		// (common convention is to multiply the base opacity by the sampled alpha from the albedo map)
		opacity *= albedo_alpha;
	}

	return opacity;
}

vec3 compute_directional_light_component(Directional_Light light, vec3 directional_light_dir, 
										 vec3 normal, vec3 view_dir)
{
	vec3 light_dir = normalize(-directional_light_dir);

	// diffuse shading
	float diff = max(dot(normal, light_dir), 0.0);

	// specular shading
	vec3 reflect_dir	= reflect(-light_dir, normal);
	float spec			= pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess);
	
	// get albedo and metallic components with fallback
	vec3 albedo_component	= get_albedo_component();
	vec3 metallic_component = get_metallic_component();

	// combine results
	vec3 ambient    = light.ambient * albedo_component;
	vec3 diffuse    = light.diffuse * diff * albedo_component;
	vec3 specular   = light.specular * spec * metallic_component;
	return (ambient + diffuse + specular);
}

vec3 compute_point_light_component(Point_Light light, vec3 point_light_pos, 
								   vec3 normal, vec3 frag_pos, vec3 view_dir)
{
	vec3 light_dir = normalize(point_light_pos - frag_pos);

	// diffuse shading
	float diff = max(dot(normal, light_dir), 0.0);

	// specular shading
	vec3 reflect_dir	= reflect(-light_dir, normal);
	float spec			= pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess);

	// attenuation
	float distance      = length(point_light_pos - frag_pos);
	float attenuation	= 1.0 / (light.constant 
						  + light.linear * distance + light.quadratic * (distance * distance));
	
	// get albedo and metallic components with fallback
	vec3 albedo_component   = get_albedo_component();
	vec3 metallic_component = get_metallic_component();

	// combine results
	vec3 ambient    = light.ambient * albedo_component;
	vec3 diffuse    = light.diffuse * diff * albedo_component;
	vec3 specular   = light.specular * spec * metallic_component;
	ambient         *= attenuation;
	diffuse         *= attenuation;
	specular        *= attenuation;
	return (ambient + diffuse + specular);
}

vec3 compute_spotlight_component(Spotlight light, vec3 spotlight_pos, vec3 spotlight_dir, 
						   vec3 normal, vec3 frag_pos, vec3 view_dir)
{
	vec3 light_dir = normalize(spotlight_pos - frag_pos);

	// diffuse shading
	float diff  = max(dot(normal, light_dir), 0.0);

	// specular shading
	vec3 reflect_dir	= reflect(-light_dir, normal);
	float spec			= pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess);

	// attenuation
	float distance      = length(spotlight_pos - frag_pos);
	float attenuation   = 1.0 / (light.constant 
						  + light.linear * distance + light.quadratic * (distance * distance));

	// spotlight intensity
	float theta     = dot(light_dir, normalize(-spotlight_dir));
	float epsilon   = light.inner_cutoff - light.outer_cutoff;
	float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

	// get albedo and metallic components with fallback
	vec3 albedo_component   = get_albedo_component();
	vec3 metallic_component = get_metallic_component();

	// combine results
	vec3 ambient    = light.ambient * albedo_component;
	vec3 diffuse    = light.diffuse * diff * albedo_component;
	vec3 specular   = light.specular * spec * metallic_component;
	ambient         *= attenuation * intensity;
	diffuse         *= attenuation * intensity;
	specular        *= attenuation * intensity;
	return (ambient + diffuse + specular);
}