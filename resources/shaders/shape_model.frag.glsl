#version 450 core
out vec4 FragColor;

// lights of the scene and lighting functions (Light_Data, Surface, compute_lighting, etc.)
#include "include/lighting.glsl"

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

// view matrix, used to transform the lights from world space to view space
uniform mat4 u_view;

// material properties struct
uniform Material u_material;

// whether the shape is drawn with instancing (albedo of the instance) or alone (albedo of the material)
uniform bool u_instanced;

void main()
{
	// albedo of the instance, or of the material when the shape is drawn alone
	vec4 albedo = u_instanced ? v_instance_albedo : u_material.albedo;

	// surface properties of the fragment (the shape is completely shiny, since it has no specular map)
	Surface surface;
	surface.position	= v_frag_pos;
	surface.normal		= normalize(v_normal);
	surface.albedo		= albedo.rgb;
	surface.specular	= vec3(1.0);
	surface.shininess	= u_material.shininess;

	// set the fragment color, lit by all the lights of the scene
	FragColor = vec4(compute_lighting(surface, u_view), albedo.a);
}
