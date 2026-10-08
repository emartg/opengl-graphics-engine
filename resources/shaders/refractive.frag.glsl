#version 450 core
out vec4 FragColor;

in vec3 v_position;
in vec3 v_normal;

// struct to hold the material parameters (set by the material, see the material descriptor files)
struct Material
{
	float refraction_ratio;	// ratio of the indices of refraction (eta_i / eta_t, e.g., 1.00 / 1.52 from air to glass)
};

uniform Material u_material;			// material parameters struct
uniform mat3 u_inv_view_rot;			// inverse of the rotation part of the view matrix
uniform samplerCube u_environment_map;	// environment map (the skybox or a dynamic cubemap, set by the renderer)

// albedo color of the object (set by the renderer for each object): its RGB components tint the refraction,
// and its alpha component is the opacity of the object
uniform vec4 u_object_albedo;

void main()
{
	// calculate the view-space incident and refraction vectors
	vec3 incident_view		= normalize(v_position); // from camera (at origin in view space) to fragment
	vec3 refraction_view	= refract(incident_view, normalize(v_normal), u_material.refraction_ratio);

	// transform the view-space refraction vector to world space for cubemap sampling
	vec3 refraction_world	= u_inv_view_rot * refraction_view;

	// sample the environment map in the refracted direction, tinted by the color of the object
	FragColor				= vec4(texture(u_environment_map, refraction_world).rgb * u_object_albedo.rgb, u_object_albedo.a);
}
