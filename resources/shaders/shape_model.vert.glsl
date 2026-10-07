#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
// per-instance attributes, only used with instanced rendering: the model matrix (which takes the
// locations 3 to 6, one per column) and the albedo of each instance
layout(location = 3) in mat4 aInstanceModel;
layout(location = 7) in vec4 aInstanceAlbedo;

// maximum number of lights in the scene (same as in the fragment shader)
#define MAX_DIR_LIGHTS_COUNT	3
#define MAX_POINT_LIGHTS_COUNT	3
#define MAX_SPOTLIGHTS_COUNT	3

// fragment position, normal and texture coordinates in u_view space to be passed to the fragment shader
out vec3 v_frag_pos;
out vec3 v_normal;
out vec2 v_tex_coords;
// albedo of the instance (only used with instanced rendering), constant across the primitive
flat out vec4 v_instance_albedo;

// statically sized arrays of light attributes in u_view space to be passed to the fragment shader
out vec3 v_directional_light_dir[MAX_DIR_LIGHTS_COUNT];
out vec3 v_point_light_pos[MAX_POINT_LIGHTS_COUNT];
out vec3 v_spotlight_pos[MAX_SPOTLIGHTS_COUNT];
out vec3 v_spotlight_dir[MAX_SPOTLIGHTS_COUNT];

// transformation matrices
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

// whether the shape is drawn with instancing (model matrix and albedo from the per-instance attributes)
// or alone (model matrix from u_model, and albedo from the material in the fragment shader)
uniform bool u_instanced;

// number of lights currently in the scene
uniform int u_directional_light_count;
uniform int u_point_light_count;
uniform int u_spotlight_count;

// statically sized arrays of light attributes in world space
uniform vec3 u_directional_light_dir[MAX_DIR_LIGHTS_COUNT];	
uniform vec3 u_point_light_pos[MAX_POINT_LIGHTS_COUNT];
uniform vec3 u_spotlight_pos[MAX_SPOTLIGHTS_COUNT];
uniform vec3 u_spotlight_dir[MAX_SPOTLIGHTS_COUNT];

void main()
{
	// loop through all directional lights and transform their attributes from world space to u_view space
	for (int i = 0; i < u_directional_light_count; i++)
	{
		v_directional_light_dir[i] = vec3(u_view * vec4(u_directional_light_dir[i], 0.0));
	}

	// loop through all point lights and transform their attributes from world space to u_view space
	for (int i = 0; i < u_point_light_count; i++)
	{
		v_point_light_pos[i] = vec3(u_view * vec4(u_point_light_pos[i], 1.0));
	}

	// loop through all spotlights and transform their attributes from world space to u_view space
	for (int i = 0; i < u_spotlight_count; i++)
	{
		v_spotlight_pos[i] = vec3(u_view * vec4(u_spotlight_pos[i], 1.0));
		// direction is a 3 component vector, so we don't need to translate it (w = 0.0)
		v_spotlight_dir[i] = vec3(u_view * vec4(u_spotlight_dir[i], 0.0));
	}

	// model matrix of the instance, or of the single shape
	mat4 model		= u_instanced ? aInstanceModel : u_model;
	v_instance_albedo = aInstanceAlbedo;

	v_frag_pos		= vec3(u_view * model * vec4(aPos, 1.0));
	v_normal		= mat3(transpose(inverse(u_view * model))) * aNormal;
	v_tex_coords	= aTexCoords;

	gl_Position		= u_projection * u_view * model * vec4(aPos, 1.0);
}
