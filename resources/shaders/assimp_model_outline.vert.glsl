#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

// outline thickness value in world space units to extrude the vertices
uniform float outlineThickness;

// transformation matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // to achieve the outline effect, we extrude the vertex position along the normal
    vec3 outlinePos = aPos + aNormal * outlineThickness;

    gl_Position = projection * view * model * vec4(outlinePos, 1.0);
}