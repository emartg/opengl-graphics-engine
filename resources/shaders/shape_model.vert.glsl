#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
// per-instance attributes, only used with instanced rendering: the model matrix (which takes the
// locations 3 to 6, one per column) and the albedo of each instance
layout(location = 3) in mat4 aInstanceModel;
layout(location = 7) in vec4 aInstanceAlbedo;

// fragment position, normal and texture coordinates in u_view space to be passed to the fragment shader
out vec3 v_frag_pos;
out vec3 v_normal;
out vec2 v_tex_coords;
// albedo of the instance (only used with instanced rendering), constant across the primitive
flat out vec4 v_instance_albedo;

// transformation matrices
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

// whether the shape is drawn with instancing (model matrix and albedo from the per-instance attributes)
// or alone (model matrix from u_model, and albedo from the material in the fragment shader)
uniform bool u_instanced;

void main()
{
	// model matrix of the instance, or of the single shape
	mat4 model		= u_instanced ? aInstanceModel : u_model;
	v_instance_albedo = aInstanceAlbedo;

	v_frag_pos		= vec3(u_view * model * vec4(aPos, 1.0));
	v_normal		= mat3(transpose(inverse(u_view * model))) * aNormal;
	v_tex_coords	= aTexCoords;

	gl_Position		= u_projection * u_view * model * vec4(aPos, 1.0);
}
