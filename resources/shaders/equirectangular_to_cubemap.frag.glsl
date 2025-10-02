#version 420 core
out vec4 FragColor;

in vec3 LocalPos;

uniform sampler2D equirectMap;

const vec2 invAtan = vec2(0.15915494, 0.31830989);

vec2 SampleSphericalMap(vec3 v);

void main()
{
    vec3 n = normalize(LocalPos);
    vec2 uv = SampleSphericalMap(n);
    vec3 hdrColor = texture(equirectMap, uv).rgb;
    FragColor = vec4(hdrColor, 1.0);
}

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}