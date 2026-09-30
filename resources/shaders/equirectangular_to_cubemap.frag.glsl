#version 450 core
out vec4 FragColor;

in vec3 v_local_pos;

uniform sampler2D u_equirect_map;

const vec2 INV_ATAN = vec2(0.15915494, 0.31830989);

// Sample the equirectangular map using spherical coordinates derived from the input vector
vec2 sample_spherical_map(vec3 v);

void main()
{
	vec3 n			= normalize(v_local_pos);
	vec2 uv			= sample_spherical_map(n);
	vec3 hdr_color	= texture(u_equirect_map, uv).rgb;
	FragColor		= vec4(hdr_color, 1.0);
}

vec2 sample_spherical_map(vec3 v)
{
	// flip the horizontal sampling to match the expected orientation
	vec2 uv = vec2(atan(v.z, -v.x), asin(v.y));
	uv		*= INV_ATAN;
	uv		+= 0.5;
	return uv;
}
