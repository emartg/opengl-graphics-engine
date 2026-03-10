#version 420 core
out vec4 FragColor;

in vec3 v_position;
in vec3 v_normal;

uniform mat3 u_inv_view_rot;	// inverse of the rotation part of the view matrix
uniform samplerCube u_skybox;	// environment map (skybox cubemap texture)

void main()
{
	// calculate the view-space incident and reflection vectors
	vec3 incident_view		= normalize(v_position); // from camera (at origin in view space) to fragment
	vec3 reflection_view	= reflect(incident_view, normalize(v_normal));

	// transform the view-space reflection vector to world space for cubemap sampling
	vec3 reflection_world	= u_inv_view_rot * reflection_view;

	// sample the environment map in the reflected direction
	FragColor				= vec4(texture(u_skybox, reflection_world).rgb, 1.0);
}