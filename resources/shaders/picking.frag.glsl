#version 420 core
out vec4 FragColor;

uniform int u_encoded_id; // 24-bit safe (node ids are reasonably small)

// Encode an integer id into an RGB color
vec3 encode_id(int id);

void main()
{
	FragColor = vec4(encode_id(u_encoded_id), 1.0);
}

vec3 encode_id(int id)
{
	// pack into RGB 0-1 range
	int r = ( id        & 0xFF);
	int g = ((id >> 8)  & 0xFF);
	int b = ((id >> 16) & 0xFF);
	return vec3(r, g, b) / 255.0;
}