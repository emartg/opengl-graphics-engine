#version 420 core
out vec4 FragColor;

in vec3 v_position;
in vec3 v_normal;

uniform float u_ratio;			// ratio of indices of refraction (eta_i / eta_t)
uniform mat3 u_inv_view_rot;	// inverse of the rotation part of the view matrix
uniform samplerCube u_skybox;	// environment map (skybox cubemap texture)

void main()
{
	// calculate the view-space incident and refraction vectors
	vec3 incident_view		= normalize(v_position); // from camera (at origin in view space) to fragment
	vec3 refraction_view	= refract(incident_view, normalize(v_normal), u_ratio);

	// transform the view-space refraction vector to world space for cubemap sampling
	vec3 refraction_world	= u_inv_view_rot * refraction_view;

	// sample the environment map in the refracted direction
	FragColor				= vec4(texture(u_skybox, refraction_world).rgb, 1.0);
}