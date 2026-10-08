#version 450 core
out vec4 FragColor;

in vec3 v_position;
in vec3 v_normal;

uniform mat3 u_inv_view_rot;			// inverse of the rotation part of the view matrix
uniform samplerCube u_environment_map;	// environment map (the skybox or a dynamic cubemap, set by the renderer)

// albedo color of the object (set by the renderer for each object): its RGB components tint the reflection,
// and its alpha component is the opacity of the object
uniform vec4 u_object_albedo;

void main()
{
	// calculate the view-space incident and reflection vectors
	vec3 incident_view		= normalize(v_position); // from camera (at origin in view space) to fragment
	vec3 reflection_view	= reflect(incident_view, normalize(v_normal));

	// transform the view-space reflection vector to world space for cubemap sampling
	vec3 reflection_world	= u_inv_view_rot * reflection_view;

	// sample the environment map in the reflected direction, tinted by the color of the object
	FragColor				= vec4(texture(u_environment_map, reflection_world).rgb * u_object_albedo.rgb, u_object_albedo.a);
}
