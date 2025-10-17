#version 420 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;		// camera position in world space
uniform samplerCube skybox; // environment map (skybox cubemap texture)

void main()
{
	// normalize input normal vector and calculate the view direction
	vec3 incident = normalize(Position - cameraPos);
	vec3 reflection = normalize(reflect(incident, normalize(Normal)));

	// sample the environment map in the reflected direction
	FragColor = vec4(texture(skybox, reflection).rgb, 1.0);
}
