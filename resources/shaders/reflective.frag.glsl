#version 420 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform mat4 view;			// view matrix (to convert view-space to world-space for cubemap sampling)
uniform samplerCube skybox; // environment map (skybox cubemap texture)

void main()
{
	// calculate the view-space incident and reflection vectors
	vec3 incidentView	 = normalize(Position); // from camera (at origin in view space) to fragment
	vec3 reflectionView  = reflect(incidentView, normalize(Normal));

	// transform the view-space reflection vector to world space for cubemap sampling
	mat3 invViewRot		 = transpose(mat3(view)); // inverse of rotation part of view matrix
	vec3 reflectionWorld = invViewRot * reflectionView;

	// sample the environment map in the reflected direction
	FragColor = vec4(texture(skybox, reflectionWorld).rgb, 1.0);
}