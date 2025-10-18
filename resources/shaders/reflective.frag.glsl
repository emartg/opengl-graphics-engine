#version 420 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform samplerCube skybox; // environment map (skybox cubemap texture)

void main()
{
	// calculate the incident vector from the fragment position to the camera 
	// (which is at the origin in view space)
	vec3 incident = normalize(FragPos);
	// normalize input normal vector and calculate the view direction
	vec3 reflection = normalize(reflect(incident, normalize(Normal)));

	// sample the environment map in the reflected direction
	FragColor = vec4(texture(skybox, reflection).rgb, 1.0);
}