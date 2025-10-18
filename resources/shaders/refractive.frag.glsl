#version 420 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform samplerCube skybox; // environment map (skybox cubemap texture)

void main()
{
	// calculate the incident vector from the fragment position to the camera 
	// (which is at the origin in view space)
	vec3 incident = normalize(Position);

	// calculate the refraction direction using Snell's law (assuming air to glass refraction)
	float ratio = 1.00 / 1.52; // air to glass refraction index ratio
	vec3 refraction = normalize(refract(incident, normalize(Normal), ratio));

	// sample the environment map in the refracted direction
	FragColor = vec4(texture(skybox, refraction).rgb, 1.0);
}