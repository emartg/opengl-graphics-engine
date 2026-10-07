#version 450 core
out vec4 FragColor;

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
	vec4 albedo;
	float shininess;
};

// fragment position, normal and texture coordinates in view space passed from the vertex shader
in vec3 v_frag_pos;
in vec3 v_normal;
in vec2 v_tex_coords;
// albedo of the instance, passed from the vertex shader (only used with instanced rendering)
flat in vec4 v_instance_albedo;

// lights of the scene, read from a shader storage buffer (binding point 0) filled by the renderer
// every frame, in world space (sorted by type: directional lights, point lights, and spotlights)
#define DIRECTIONAL_LIGHT	0
#define POINT_LIGHT			1
#define SPOTLIGHT			2

struct Light_Data
{
	vec4 position;		// xyz: position (point lights and spotlights), w: light type
	vec4 direction;		// xyz: direction (directional lights and spotlights)
	vec4 ambient;		// rgb: ambient component
	vec4 diffuse;		// rgb: diffuse component
	vec4 specular;		// rgb: specular component
	vec4 attenuation;	// x: constant, y: linear, z: quadratic factor (point lights and spotlights)
	vec4 cutoffs;		// x: inner, y: outer cut-off (spotlights)
};

layout(std430, binding = 0) readonly buffer Light_Buffer
{
	int light_count;		// number of lights in the scene
	Light_Data lights[];	// lights of the scene (as many as light_count)
};

// view matrix, used to transform the lights from world space to view space
uniform mat4 u_view;

// material properties struct
uniform Material u_material;

// whether the shape is drawn with instancing (albedo of the instance) or alone (albedo of the material)
uniform bool u_instanced;

// albedo of the fragment, set at the start of main() (from the instance or from the material)
vec4 albedo;

// Calculates the color of a single directional light given the light properties 
// (including the direction), the normal, and the view direction (all in view space)
vec3 compute_directional_light_component(Directional_Light light, vec3 directional_light_dir, 
										 vec3 normal, vec3 view_dir);

// Calculates the color of a single point light given the light properties (including the position),
// the fragment position, the normal, and the view direction (all in view space)
vec3 compute_point_light_component(Point_Light light, vec3 point_light_pos, 
								   vec3 normal, vec3 frag_pos, vec3 view_dir);

// Canculates the color of a single spotlight given the light properties 
// (including the position and direction), the fragment position, the normal, 
// and the view direction (all in view space)
vec3 compute_spotlight_component(Spotlight light, vec3 spotlight_pos, vec3 spotlight_dir, 
								 vec3 normal, vec3 frag_pos, vec3 view_dir);

void main()
{
	// albedo of the instance, or of the material when the shape is drawn alone
	albedo = u_instanced ? v_instance_albedo : u_material.albedo;

	// light properties
	vec3 normal     = normalize(v_normal);
	vec3 view_dir	= normalize(-v_frag_pos);	// since lighting is being calculated in view space,
												// viewPos is (0, 0, 0)

	// initialize fragment color
	vec3 result = vec3(0.0);

	// loop through all the lights and accumulate their contributions, transforming their attributes
	// from world space to view space (directions are not translated, so their w component is 0.0)
	for (int i = 0; i < light_count; i++)
	{
		Light_Data light = lights[i];
		int light_type   = int(light.position.w);

		if (light_type == DIRECTIONAL_LIGHT)
		{
			Directional_Light directional_light = Directional_Light(light.ambient.rgb, light.diffuse.rgb, light.specular.rgb);
			vec3 light_dir = vec3(u_view * vec4(light.direction.xyz, 0.0));
			result += compute_directional_light_component(directional_light, light_dir, normal, view_dir);
		}
		else if (light_type == POINT_LIGHT)
		{
			Point_Light point_light = Point_Light(light.ambient.rgb, light.diffuse.rgb, light.specular.rgb,
												  light.attenuation.x, light.attenuation.y, light.attenuation.z);
			vec3 light_pos = vec3(u_view * vec4(light.position.xyz, 1.0));
			result += compute_point_light_component(point_light, light_pos, normal, v_frag_pos, view_dir);
		}
		else if (light_type == SPOTLIGHT)
		{
			Spotlight spotlight = Spotlight(light.ambient.rgb, light.diffuse.rgb, light.specular.rgb,
											light.attenuation.x, light.attenuation.y, light.attenuation.z,
											light.cutoffs.x, light.cutoffs.y);
			vec3 light_pos = vec3(u_view * vec4(light.position.xyz, 1.0));
			vec3 light_dir = vec3(u_view * vec4(light.direction.xyz, 0.0));
			result += compute_spotlight_component(spotlight, light_pos, light_dir, normal, v_frag_pos, view_dir);
		}
	}

	// set the fragment color
	FragColor	= vec4(result, albedo.a);
}

vec3 compute_directional_light_component(Directional_Light light, vec3 directional_light_dir, 
										 vec3 normal, vec3 view_dir)
{
	vec3 light_dir	= normalize(-directional_light_dir);

	// diffuse shading
	float diff		= max(dot(normal, light_dir), 0.0);

	// specular shading
	vec3 reflect_dir	= reflect(-light_dir, normal);
	float spec			= pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess);

	// get RGB components of albedo
	vec3 albedo_rgb	= albedo.rgb;
	
	// combine results
	vec3 ambient    = light.ambient * albedo_rgb;
	vec3 diffuse    = light.diffuse * diff * albedo_rgb;
	vec3 specular   = light.specular * spec; // the object is completely shiny since there is no specular map
	return (ambient + diffuse + specular);
}

vec3 compute_point_light_component(Point_Light light, vec3 point_light_pos, 
								   vec3 normal, vec3 frag_pos, vec3 view_dir)
{
	vec3 light_dir	= normalize(point_light_pos - frag_pos);

	// diffuse shading
	float diff		= max(dot(normal, light_dir), 0.0);

	// specular shading
	vec3 reflect_dir	= reflect(-light_dir, normal);
	float spec			= pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess);

	// attenuation
	float distance      = length(point_light_pos - frag_pos);
	float attenuation   = 1.0 / (light.constant + light.linear * distance 
						  + light.quadratic * (distance * distance));

	// get RGB components of albedo
	vec3 albedo_rgb = albedo.rgb;

	// combine results
	vec3 ambient    = light.ambient * albedo_rgb;
	vec3 diffuse    = light.diffuse * diff * albedo_rgb;
	vec3 specular   = light.specular * spec; // the object is completely shiny since there is no specular map
	ambient         *= attenuation;
	diffuse         *= attenuation;
	specular        *= attenuation;
	return (ambient + diffuse + specular);
}

vec3 compute_spotlight_component(Spotlight light, vec3 spotlight_pos, vec3 spotlight_dir, 
						   vec3 normal, vec3 frag_pos, vec3 view_dir)
{
	vec3 light_dir	= normalize(spotlight_pos - frag_pos);

	// diffuse shading
	float diff		= max(dot(normal, light_dir), 0.0);

	// specular shading
	vec3 reflect_dir	= reflect(-light_dir, normal);
	float spec			= pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess);

	// attenuation
	float distance      = length(spotlight_pos - frag_pos);
	float attenuation   = 1.0 / (light.constant + light.linear * distance 
						  + light.quadratic * (distance * distance));

	// spotlight intensity
	float theta     = dot(light_dir, normalize(-spotlight_dir));
	float epsilon   = light.inner_cutoff - light.outer_cutoff;
	float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

	// get RGB components of albedo
	vec3 albedo_rgb = albedo.rgb;

	// combine results
	vec3 ambient    = light.ambient * albedo_rgb;
	vec3 diffuse    = light.diffuse * diff * albedo_rgb;
	vec3 specular   = light.specular * spec; // the object is completely shiny since there is no specular map
	ambient         *= attenuation * intensity;
	diffuse         *= attenuation * intensity;
	specular        *= attenuation * intensity;
	return (ambient + diffuse + specular);
}
