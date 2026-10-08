#version 450 core
out vec4 FragColor;

// lights of the scene and lighting functions (Light_Data, Surface, compute_lighting, etc.)
#include "include/lighting.glsl"

// struct to hold the material parameters and textures (set by the material, see the material descriptor files):
// each texture slot of the shader descriptor (albedo, metallic, and opacity maps) has a sampler and a flag that
// tells whether the material has a texture for it
struct Material
{
	float shininess;

	sampler2D albedo_map;	// albedo (diffuse) map, which tints the albedo color of the object
	sampler2D metallic_map;	// metallic (specular) map, which tints the specular highlights
	sampler2D opacity_map;	// opacity (alpha) map, read from its red channel

	bool has_albedo_map;
	bool has_metallic_map;
	bool has_opacity_map;
};

// fragment position, normal and texture coordinates in view space passed from the vertex shader
in vec3 v_frag_pos;
in vec3 v_normal;
in vec2 v_tex_coords;
// albedo of the instance, passed from the vertex shader (only used with instanced rendering)
flat in vec4 v_instance_albedo;

// view matrix, used to transform the lights from world space to view space
uniform mat4 u_view;

// material parameters and textures struct
uniform Material u_material;

// albedo color of the object (set by the renderer for each object drawn alone): its RGB components tint the
// material, and its alpha component is the base opacity of the object
uniform vec4 u_object_albedo;

// whether the object is drawn with instancing (albedo of the instance) or alone (albedo of the object)
uniform bool u_instanced;

// Helper functions to fetch the material's maps, with a fallback if the material has no texture for them
vec3 get_albedo_component();
vec3 get_metallic_component();
float get_opacity_component(float base_opacity);

void main()
{
	// albedo color of the instance, or of the object when it is drawn alone
	vec4 object_albedo = u_instanced ? v_instance_albedo : u_object_albedo;

	// surface properties of the fragment: the albedo of the object tinted by the albedo map, and the specular
	// color of the metallic map (white, i.e., completely shiny, without one)
	Surface surface;
	surface.position	= v_frag_pos;
	surface.normal		= normalize(v_normal);
	surface.albedo		= object_albedo.rgb * get_albedo_component();
	surface.specular	= get_metallic_component();
	surface.shininess	= u_material.shininess;

	// lighting result of all the lights of the scene
	vec3 result = compute_lighting(surface, u_view);

	// get the final opacity from the opacity of the object and the material's maps (if any)
	float opacity = get_opacity_component(object_albedo.a);

	// discard the nearly transparent fragments of the textures with transparency (e.g., the cut-out parts of a
	// leaf), to improve performance and avoid blending issues (the opacity of the object alone always blends)
	if ((u_material.has_opacity_map || u_material.has_albedo_map) && opacity < 0.1) discard;

	// set the fragment color with the computed lighting result and the final opacity
	FragColor = vec4(result, opacity);
}

vec3 get_albedo_component()
{
	vec3 sampled_color = texture(u_material.albedo_map, v_tex_coords).rgb;
	// fallback to white so the albedo color of the object is used as is if there is no albedo map
	return mix(vec3(1.0), sampled_color, float(u_material.has_albedo_map));
}

vec3 get_metallic_component()
{
	vec3 sampled_color = texture(u_material.metallic_map, v_tex_coords).rgb;
	// fallback to white so specular highlights remain visible if no metallic map is used
	return mix(vec3(1.0), sampled_color, float(u_material.has_metallic_map));
}

float get_opacity_component(float base_opacity)
{
	float opacity = base_opacity; // default opacity from the base opacity of the object

	// if an opacity map is used, fetch the opacity from the red channel of the texture
	// (convention for opacity maps is to store opacity in the red channel)
	// and combine it with the base opacity for overall control
	if (u_material.has_opacity_map)
	{
		// sample the red channel of the opacity map to get the opacity value
		opacity = texture(u_material.opacity_map, v_tex_coords).r;
		// combine the sampled opacity with the base opacity to get the final opacity value for the fragment
		// (common convention is to multiply the base opacity by the sampled opacity from the opacity map)
		opacity *= base_opacity;
	}
	// if no opacity map is used but an albedo map is used,
	// the alpha channel of the albedo map can be used for opacity (if it exists in the texture);
	// and in any case, the alpha channel is combined with the base opacity for overall control
	else if (u_material.has_albedo_map)
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
