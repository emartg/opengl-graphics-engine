#version 420 core
out vec4 FragColor;

// struct to hold screen debug mode parameters
struct Screen_Debug_Params
{
	int debug_mode;				// current debug mode (0 = normal, 1 = inverted colors, 
								//					   2 = picking colors, 3 = solid color, 4 = grid overlay)
	vec3 solid_color;			// color to use when the debug mode is 1
	int grid_line_count;		// number of lines in the grid overlay when the debug mode is 2
	float grid_line_thickness;	// thickness of the lines in the grid overlay when the debug mode is 2
	vec3 grid_bg_color;			// background color for the grid overlay when the debug mode is 2
	vec3 grid_line_color;		// line color for the grid overlay when the debug mode is 2
};

// texture coordinates in clip space passed from the vertex shader
in vec2 v_tex_coords;

// screen dimensions and screen texture to be sampled
uniform vec2 u_screen_size;
uniform sampler2D u_screen_texture; // regular scene texture or picking visualization texture

// outline mask texture (alpha channel = 1.0 inside object silhouette, 0.0 outside)
uniform sampler2D u_outline_mask_texture;

// outline uniforms (used if the screen texture has an outline mask)
uniform int u_has_outline;			// whether the screen texture has an outline
									// (1 if outline mask is present)
uniform vec3 u_outline_color;		// color to use for the outline if present
uniform int u_outline_thickness;	// thickness of the outline in pixels (radius)

// depth-aware outline uniforms
uniform int u_has_depth_textures;			// 1 if depth textures are bound (scene and selected object 
											// depth textures), 0 otherwise (no depth-aware outline)
uniform sampler2D u_scene_depth_texture;	// full screen depth texture of the scene
uniform sampler2D u_outline_depth_texture;	// depth texture of the selected object, 
											// i.e. where the outline mask > 0
uniform float u_outline_depth_bias;			// depth bias to apply when comparing depths 
											// for depth-aware outline, avoiding z-fighting artifacts

// screen debug mode parameters to control the rendering mode
uniform Screen_Debug_Params u_screen_debug_params;

// Creates a grid overlay on the UV coordinates with the screen size and specified parameters
vec3 grid(vec2 uv, int grid_line_count, float grid_line_thickness, vec3 grid_bg_color, vec3 grid_line_color);

// Helper to decode id from raw encoded RGB (must mirror picking.frag.glsl)
uint decode_id(vec3 c);

// Creates a pseudo-random color in [0,1] from an integer id by hashing it (must mirror picking.frag.glsl)
vec3 hash_id(uint id);

// Boosts the brightness of dark colors using a gamma correction
vec3 boost(vec3 c);

// Outputs the outline mask value at the given UV coordinates (1.0 = inside object silhouette, 0.0 = outside)
float sample_mask(vec2 uv);

void main()
{
	vec4 base_color;

	if (u_screen_debug_params.debug_mode == 0)
	{ // normal rendering: sample the screen texture
		base_color		= texture(u_screen_texture, v_tex_coords);
		base_color.a	= 1.0; // ensure alpha is 1.0 to prevent transparency issues
	}
	else if (u_screen_debug_params.debug_mode == 1)
	{ // inverted color rendering: sample the screen texture and invert the color
		vec4 color	= texture(u_screen_texture, v_tex_coords);
		base_color	= vec4(1.0 - color.rgb, 1.0);
	}
	else if (u_screen_debug_params.debug_mode == 2)
	{ // picking color rendering: sample the screen texture and decode the id to a unique color per object
		vec3 raw	= texture(u_screen_texture, v_tex_coords).rgb; // raw encoded id color from picking pass
		uint id		= decode_id(raw); // decode the id (0 = no object)
		if (id == 0)
		{ 
			// if no object, render black
			base_color = vec4(0.0, 0.0, 0.0, 1.0);
		}
		else
		{ 
			// if object, render a boosted color based on the hashed id to get a unique but consistent
			// color for each object
			vec3 hashed = boost(hash_id(id)); // boost to make dark colors more visible
			base_color	= vec4(hashed, 1.0);
		}
	}
	else if (u_screen_debug_params.debug_mode == 3)
	{ // solid color rendering: use the solid color specified in the debug mode parameters
		base_color = vec4(u_screen_debug_params.solid_color, 1.0);
	}
	else if (u_screen_debug_params.debug_mode == 4)
	{ // grid overlay rendering: create a grid overlay on the UV coordinates with the specified parameters
		int count		= max(u_screen_debug_params.grid_line_count, 1); // at least 1 line
		float thickness = max(u_screen_debug_params.grid_line_thickness, 1.0f); // at least 1 px thickness
		// compute the grid color based on the texture coords as UV coords and other parameters
		vec3 color		= grid(v_tex_coords, count, thickness, 
							   u_screen_debug_params.grid_bg_color, u_screen_debug_params.grid_line_color);
		base_color		= vec4(color, 1.0);
	}
	else
	{ // unrecognized debug_mode: default to black solid color
		base_color = vec4(0.0, 0.0, 0.0, 1.0);
	}

	// if no outline is not present, invalid, or the debug mode does not support it
	// (i.e. all the modes except normal rendering and inverted colors), just output the base color as is
	if (u_has_outline == 0 || u_outline_thickness <= 0 || u_screen_debug_params.debug_mode >= 2)
		FragColor = base_color;

	// outline rendering: apply an outline effect based on the outline mask in the alpha channel
	float mask = sample_mask(v_tex_coords); // sample the outline mask at the current pixel

	// only draw outline on pixels outside the object silhouette (outside edge outline)
	if (mask > 0.0)
	{ // if inside the object silhouette: output the base color as is and return early
		FragColor = base_color;
		return;
	}

	// compute the outline by checking the surrounding pixels within the outline thickness radius
	vec2 texel = 1.0 / u_screen_size;	// size of one texel in UV coordinates
	int radius = u_outline_thickness;	// outline radius in pixels

	// check the surrounding pixels in a square area of (2 * radius + 1) x ( 2 * radius + 1)
	bool edge = false; // flag to indicate if an edge pixel is found in the surrounding area
	for (int dy = -radius; dy <= radius && !edge; ++dy)
	{ // break the outer loop if an edge pixel is found
		for (int dx = -radius; dx <= radius; ++dx)
		{ // check each pixel in the square area
			vec2 offset = vec2(float(dx), float(dy)) * texel; // offset in UV coordinates
			if (sample_mask(v_tex_coords + offset) > 0.0) // sample the outline mask at the offset position
			{ // if any surrounding pixel is inside the object silhouette, mark as edge
				edge = true;
				break; // break the inner loop, since we only need one edge pixel
			}
		}
	}

	// output the depth-aware outline color if an edge pixel was found, otherwise output the base color
	if (edge)
	{ // depth-aware: only draw outline if selected surface is in front of current pixel's scene depth
		if (u_has_depth_textures == 1)
		{ // if depth textures are available, perform depth-aware outline rendering
			// sample the scene depth at the current pixel
			float scene_depth		= texture(u_scene_depth_texture, v_tex_coords).r;
			// find representative depth of selected object near the edge,
			// sampling again in a small neighborhood around the pixel 
			// and picking the minimum (closest) depth, where the outline mask > 0
			float selected_depth	= 1.0; // initialize to far plane depth (1.0 in [0,1] depth range)
			// search in a smaller radius to avoid bleeding of the outline depth
			for (int dy = -radius; dy <= radius; ++dy)
			{ // loop over y offsets
				for (int dx = -radius; dx <= radius; ++dx)
				{ // loop over x offsets
					vec2 offset			= vec2(float(dx), float(dy)) * texel; // offset in UV coordinates
					// sample the outline mask at the offset pos to check if inside the object silhouette
					float outline_mask	= sample_mask(v_tex_coords + offset);
					if (outline_mask > 0.0)
					{ // if inside the object silhouette, sample the selected object depth texture
						float depth		= texture(u_outline_depth_texture, v_tex_coords + offset).r;
						selected_depth	= min(selected_depth, depth); // keep the minimum (closest) depth
					}
				}
			}
			// draw outline only if selected surface is in front (smaller depth value) by a small bias
			// to avoid z-fighting artifacts, otherwise draw the base color
			if (selected_depth + u_outline_depth_bias < scene_depth)
				FragColor = vec4(u_outline_color, 1.0);
			else
				FragColor = base_color;
		}
		else // fallback if depth textures are unavailable
			FragColor = vec4(u_outline_color, 1.0);
	}
	else
		FragColor = base_color;
}

vec3 grid(vec2 uv, int grid_line_count, float grid_line_thickness, vec3 grid_bg_color, vec3 grid_line_color)
{
	// convert UV coordinates to pixels using the screen size
	vec2 pixel_coords	= uv * u_screen_size;

	// calculate the cell size in pixels (non-integer values are allowed)
	float cell_width	= u_screen_size.x / float(grid_line_count);
	float cell_height	= u_screen_size.y / float(grid_line_count);
	// calculate the position within the cell in pixels
	float cell_pos_x	= mod(pixel_coords.x, cell_width);
	float cell_pos_y	= mod(pixel_coords.y, cell_height);

	// compute the distance to the nearest vertical/horizontal grid line
	float dist_to_vert_line = min(cell_pos_x, cell_width - cell_pos_x);
	float dist_to_hor_line	= min(cell_pos_y, cell_height - cell_pos_y);
	float dist_to_line		= min(dist_to_vert_line, dist_to_hor_line);

	// ensure at least 0.5 pixel half-thickness (i.e. 1 pixel full thickness)
	float half_thickness	= max(grid_line_thickness * 0.5, 0.5);

	// anti-alias the grid lines based on the derivative of the pixel coordinates
	float as_smoothness = 1.0;
	float mask			= 1.0 - smoothstep(half_thickness - as_smoothness,
						half_thickness + as_smoothness, dist_to_line);

	// mix the background color and the line color based on the mask (ensure the mask is between 0 and 1)
	return mix(grid_bg_color, grid_line_color, clamp(mask, 0.0, 1.0));
}

uint decode_id(vec3 c)
{
	uint r = uint(round(c.r * 255.0));
	uint g = uint(round(c.g * 255.0));
	uint b = uint(round(c.b * 255.0));
	return r | (g << 8) | (b << 16);
}

vec3 hash_id(uint id)
{
	uint hash = id;
	hash	^= hash >> 16;
	hash	*= 0x7feb352dU;
	hash	^= hash >> 15;
	hash	*= 0x846ca68bU;
	hash	^= hash >> 16;
	uint r	= (hash      ) & 0xFFu;
	uint g	= (hash >>  8) & 0xFFu;
	uint b	= (hash >> 16) & 0xFFu;
	return vec3(r, g, b) / 255.0;
}

vec3 boost(vec3 c)
{
	return pow(c, vec3(0.8)); // 0.8 gamma = brighten dark colors
}

float sample_mask(vec2 uv)
{
	return texture(u_outline_mask_texture, uv).r;
}