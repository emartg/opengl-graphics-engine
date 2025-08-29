#version 420 core
out vec4 FragColor;

uniform int encodedId; // 24-bit safe (Asset ids reasonably small)

// Encode an integer ID into an RGB color
vec3 encodeId(int id)
{
    // pack into RGB 0-1 range
    int r = ( id        & 0xFF);
    int g = ((id >> 8)  & 0xFF);
    int b = ((id >> 16) & 0xFF);
    return vec3(r, g, b) / 255.0;
}

void main()
{
    FragColor = vec4(encodeId(encodedId), 1.0);
}