#version 450 core
out vec4 FragColor;

in vec3 v_tex_coords;

uniform samplerCube u_skybox;

void main()
{    
	vec3 albedo_rgb = texture(u_skybox, v_tex_coords).rgb;
	FragColor		= vec4(albedo_rgb, 1.0); // force alpha to 1.0 to prevent transparency issues
}