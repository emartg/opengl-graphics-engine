#version 420 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
	vec3 colorRGB = texture(skybox, TexCoords).rgb;
	FragColor = vec4(colorRGB, 1.0); // force alpha to 1.0 to prevent transparency issues
}