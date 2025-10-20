#version 420 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform mat4 view;			// view matrix (to convert view-space to world-space for cubemap sampling)
uniform samplerCube skybox; // environment map (skybox cubemap texture)

void main()
{
	// calculate the view-space incident and refraction vectors
	vec3 incidentView	 = normalize(Position); // from fragment to camera (at origin in view space)
	float ratio			 = 1.00 / 1.52; // air to glass refraction index ratio
	vec3 refractionView  = refract(incidentView, normalize(Normal), ratio);

	// transform the view-space refraction vector to world space for cubemap sampling
	mat3 invViewRot		 = transpose(mat3(view)); // inverse of rotation part of view matrix
	vec3 refractionWorld = invViewRot * refractionView;

	// sample the environment map in the refracted direction
	FragColor = vec4(texture(skybox, refractionWorld).rgb, 1.0);
}