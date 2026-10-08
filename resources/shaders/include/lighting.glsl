// lighting.glsl
// Lighting shared by the lit shaders, which include it with #include "include/lighting.glsl" (resolved by
// the engine's shader preprocessor): the lights of the scene, read from the light buffer, and the Phong
// lighting model of directional lights, point lights, and spotlights, computed in view space

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

// properties of the surface at a fragment, needed to light it (position and normal in view space)
struct Surface
{
	vec3 position;		// position of the fragment in view space
	vec3 normal;		// normalized normal in view space
	vec3 albedo;		// diffuse color (also used for the ambient component)
	vec3 specular;		// specular color (white for a completely shiny surface)
	float shininess;	// specular exponent
};

// Calculates the color of a single directional light given its light direction
// (from the light towards the scene), the surface, and the view direction (all in view space)
vec3 compute_directional_light_component(Light_Data light, vec3 directional_light_dir, Surface surface, vec3 view_dir)
{
	vec3 light_dir	= normalize(-directional_light_dir);

	// diffuse shading
	float diff		= max(dot(surface.normal, light_dir), 0.0);

	// specular shading
	vec3 reflect_dir	= reflect(-light_dir, surface.normal);
	float spec			= pow(max(dot(view_dir, reflect_dir), 0.0), surface.shininess);

	// combine results
	vec3 ambient    = light.ambient.rgb * surface.albedo;
	vec3 diffuse    = light.diffuse.rgb * diff * surface.albedo;
	vec3 specular   = light.specular.rgb * spec * surface.specular;
	return (ambient + diffuse + specular);
}

// Calculates the color of a single point light given its position, the surface,
// and the view direction (all in view space)
vec3 compute_point_light_component(Light_Data light, vec3 point_light_pos, Surface surface, vec3 view_dir)
{
	vec3 light_dir	= normalize(point_light_pos - surface.position);

	// diffuse shading
	float diff		= max(dot(surface.normal, light_dir), 0.0);

	// specular shading
	vec3 reflect_dir	= reflect(-light_dir, surface.normal);
	float spec			= pow(max(dot(view_dir, reflect_dir), 0.0), surface.shininess);

	// attenuation
	float distance      = length(point_light_pos - surface.position);
	float attenuation   = 1.0 / (light.attenuation.x + light.attenuation.y * distance
						  + light.attenuation.z * (distance * distance));

	// combine results
	vec3 ambient    = light.ambient.rgb * surface.albedo;
	vec3 diffuse    = light.diffuse.rgb * diff * surface.albedo;
	vec3 specular   = light.specular.rgb * spec * surface.specular;
	ambient         *= attenuation;
	diffuse         *= attenuation;
	specular        *= attenuation;
	return (ambient + diffuse + specular);
}

// Calculates the color of a single spotlight given its position and direction, the surface,
// and the view direction (all in view space)
vec3 compute_spotlight_component(Light_Data light, vec3 spotlight_pos, vec3 spotlight_dir, Surface surface, vec3 view_dir)
{
	vec3 light_dir	= normalize(spotlight_pos - surface.position);

	// diffuse shading
	float diff		= max(dot(surface.normal, light_dir), 0.0);

	// specular shading
	vec3 reflect_dir	= reflect(-light_dir, surface.normal);
	float spec			= pow(max(dot(view_dir, reflect_dir), 0.0), surface.shininess);

	// attenuation
	float distance      = length(spotlight_pos - surface.position);
	float attenuation   = 1.0 / (light.attenuation.x + light.attenuation.y * distance
						  + light.attenuation.z * (distance * distance));

	// spotlight intensity (smooth transition between the inner and outer cut-offs)
	float theta     = dot(light_dir, normalize(-spotlight_dir));
	float epsilon   = light.cutoffs.x - light.cutoffs.y;
	float intensity = clamp((theta - light.cutoffs.y) / epsilon, 0.0, 1.0);

	// combine results
	vec3 ambient    = light.ambient.rgb * surface.albedo;
	vec3 diffuse    = light.diffuse.rgb * diff * surface.albedo;
	vec3 specular   = light.specular.rgb * spec * surface.specular;
	ambient         *= attenuation * intensity;
	diffuse         *= attenuation * intensity;
	specular        *= attenuation * intensity;
	return (ambient + diffuse + specular);
}

// Calculates the color of the surface lit by all the lights of the scene, given the view matrix
// (used to transform the lights from world space to view space, where the camera is at the origin)
vec3 compute_lighting(Surface surface, mat4 view)
{
	vec3 view_dir = normalize(-surface.position);

	// accumulate the contribution of every light (directions are not translated, so their w component is 0.0)
	vec3 result = vec3(0.0);
	for (int i = 0; i < light_count; i++)
	{
		Light_Data light = lights[i];
		int light_type   = int(light.position.w);

		if (light_type == DIRECTIONAL_LIGHT)
		{
			vec3 light_dir = vec3(view * vec4(light.direction.xyz, 0.0));
			result += compute_directional_light_component(light, light_dir, surface, view_dir);
		}
		else if (light_type == POINT_LIGHT)
		{
			vec3 light_pos = vec3(view * vec4(light.position.xyz, 1.0));
			result += compute_point_light_component(light, light_pos, surface, view_dir);
		}
		else if (light_type == SPOTLIGHT)
		{
			vec3 light_pos = vec3(view * vec4(light.position.xyz, 1.0));
			vec3 light_dir = vec3(view * vec4(light.direction.xyz, 0.0));
			result += compute_spotlight_component(light, light_pos, light_dir, surface, view_dir);
		}
	}
	return result;
}
