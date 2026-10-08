#version 450 core
out vec4 FragColor;

// lights of the scene and lighting functions (Light_Data, Surface, compute_lighting, etc.)
#include "include/lighting.glsl"

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
};

// fragment position, normal and texture coordinates in view space passed from the vertex shader
in vec3 v_frag_pos;
in vec3 v_normal;
in vec2 v_tex_coords;

// view matrix, used to transform the lights from world space to view space
uniform mat4 u_view;

// material properties struct
uniform Material u_material;

// albedo color of the object (set by the renderer for each object): its alpha is the base opacity of the model
uniform vec4 u_object_albedo;

// Helper functions to fetch with fallback if no texture is used
vec3 get_albedo_component();
vec3 get_metallic_component();
float get_opacity_component();

void main()
{
	// surface properties of the fragment, with the albedo and specular colors of its maps (if any)
	Surface surface;
	surface.position	= v_frag_pos;
	surface.normal		= normalize(v_normal);
	surface.albedo		= get_albedo_component();
	surface.specular	= get_metallic_component();
	surface.shininess	= u_material.shininess;

	// lighting result of all the lights of the scene
	vec3 result = compute_lighting(surface, u_view);

	// get final opacity either from the opacity map (if it exists) or from the base opacity of the object
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
	float opacity = u_object_albedo.a; // default opacity from the base opacity of the object

	// if an opacity map is used, fetch the opacity from the red channel of the texture
	// (convention for opacity maps is to store opacity in the red channel) 
	// and combine it with the base opacity for overall control
	if (u_material.has_opacity_map == 1)
	{
		// sample the red channel of the opacity map to get the opacity value
		opacity = texture(u_material.opacity_map, v_tex_coords).r;
		// combine the sampled opacity with the base opacity to get the final opacity value for the fragment
		// (common convention is to multiply the base opacity by the sampled opacity from the opacity map)
		opacity *= u_object_albedo.a;
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

