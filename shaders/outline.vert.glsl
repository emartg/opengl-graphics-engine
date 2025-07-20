#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform float outlineThickness;
uniform float outlineScaleFactor; // scale factor used to adjust outline thickness
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // adjust the outline thickness using the scale factor
    float adjustedOutlineThickness = outlineThickness * outlineScaleFactor;
    // to achieve the outline effect, we extrude the vertex position along the normal
    vec3 outlinePos = aPos + aNormal * outlineThickness;

    gl_Position = projection * view * model * vec4(outlinePos, 1.0);
}