#version 420 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform float ratio;		// ratio of indices of refraction (eta_i / eta_t)
uniform mat3 invViewRot;	// inverse of the rotation part of the view matrix
uniform samplerCube skybox; // environment map (skybox cubemap texture)

void main()
{
	// calculate the view-space incident and refraction vectors
	vec3 incidentView	 = normalize(Position); // from fragment to camera (at origin in view space)
	vec3 refractionView  = refract(incidentView, normalize(Normal), ratio);

	// transform the view-space refraction vector to world space for cubemap sampling
	vec3 refractionWorld = invViewRot * refractionView;

	// sample the environment map in the refracted direction
	FragColor = vec4(texture(skybox, refractionWorld).rgb, 1.0);
}