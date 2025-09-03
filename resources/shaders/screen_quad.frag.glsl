#version 420 core
out vec4 FragColor;

// struct to hold screen debug mode parameters
struct ScreenDebugParams
{
	int debugMode;				// current debug mode (0 = normal, 1 = inverted colors, 
								//					   2 = picking colors, 3 = solid color, 4 = grid overlay)
	vec3 solidColor;			// color to use when the debug mode is 1
	int gridLineCount;			// number of lines in the grid overlay when the debug mode is 2
	float gridLineThickness;	// thickness of the lines in the grid overlay when the debug mode is 2
	vec3 gridBgColor;			// background color for the grid overlay when the debug mode is 2
	vec3 gridLineColor;			// line color for the grid overlay when the debug mode is 2
};

// texture coordinates in clip space passed from the vertex shader
in vec2 TexCoords;

// screen dimensions and screen texture to be sampled
uniform vec2 screenSize;
uniform sampler2D screenTexture; // regular scene texture or picking visualization texture

// outline mask texture (alpha channel = 1.0 inside object silhouette, 0.0 outside)
uniform sampler2D outlineMaskTexture;

// outline parameters (used if the screen texture has an outline mask)
uniform int hasOutline;			// whether the screen texture has an outline (1 if outline mask is present)
uniform vec3 outlineColor;		// color to use for the outline if present
uniform int outlineThickness;	// thickness of the outline in pixels (radius)

// screen debug mode parameters to control the rendering mode
uniform ScreenDebugParams screenDebugParams;

// Creates a grid overlay on the UV coordinates with the screen size and specified parameters
vec3 grid(vec2 uv, int gridLineCount, float gridLineThickness, vec3 gridBgColor, vec3 gridLineColor);

// Helper to decode id from raw encoded RGB (must mirror picking.frag.glsl)
uint decodeId(vec3 c);

// Creates a pseudo-random color in [0,1] from an integer id by hashing it (must mirror picking.frag.glsl)
vec3 hashId(uint id);

// Boosts the brightness of dark colors using a gamma correction
vec3 boost(vec3 c);

// Outputs the outline mask value at the given UV coordinates (1.0 = inside object silhouette, 0.0 = outside)
float sampleMask(vec2 uv);

void main()
{
	vec4 baseColor;

	if (screenDebugParams.debugMode == 0)
	{ // normal rendering: sample the screen texture
		baseColor = texture(screenTexture, TexCoords);
	}
	else if (screenDebugParams.debugMode == 1)
	{ // inverted color rendering: sample the screen texture and invert the color
		vec4 color = texture(screenTexture, TexCoords);
		baseColor = vec4(1.0 - color.rgb, 1.0);
	}
	else if (screenDebugParams.debugMode == 2)
	{ // picking color rendering: sample the screen texture and decode the id to a unique color per object
		vec3 raw = texture(screenTexture, TexCoords).rgb; // raw encoded id color from picking pass
		uint id = decodeId(raw); // decode the id (0 = no object)
		if (id == 0u)
		{ 
			// if no object, render black
			baseColor = vec4(0.0, 0.0, 0.0, 1.0);
		}
		else
		{ 
			// if object, render a boosted color based on the hashed id to get a unique but consistent
			// color for each object
			vec3 hashed = boost(hashId(id)); // boost to make dark colors more visible
			baseColor = vec4(hashed, 1.0);
		}
	}
	else if (screenDebugParams.debugMode == 3)
	{ // solid color rendering: use the solid color specified in the debug mode parameters
		baseColor = vec4(screenDebugParams.solidColor, 1.0);
	}
	else if (screenDebugParams.debugMode == 4)
	{ // grid overlay rendering: create a grid overlay on the UV coordinates with the specified parameters
		int count = max(screenDebugParams.gridLineCount, 1); // ensure at least 1 line
		float thickness = max(screenDebugParams.gridLineThickness, 1.0f); // ensure at least 1 px thickness
		// compute the grid color based on the texture coords as UV coords and other parameters
		vec3 color = grid(TexCoords, count, thickness, 
						  screenDebugParams.gridBgColor, screenDebugParams.gridLineColor);
		baseColor = vec4(color, 1.0);
	}
	else
	{ // unrecognized debugMode: default to black solid color
		baseColor = vec4(0.0, 0.0, 0.0, 1.0);
	}

	// if no outline is not present, invalid, or the debug mode does not support it
	// (i.e. all the modes except normal rendering and inverted colors), just output the base color as is
	if (hasOutline == 0 || outlineThickness <= 0 || screenDebugParams.debugMode >= 2)
		FragColor = baseColor;

	// outline rendering: apply an outline effect based on the outline mask in the alpha channel
	float mask = sampleMask(TexCoords); // sample the outline mask at the current pixel

	// only draw outline on pixels outside the object silhouette (outside edge outline)
	if (mask > 0.0)
	{ // if inside the object silhouette: output the base color as is and return early
		FragColor = baseColor;
		return;
	}

	// compute the outline by checking the surrounding pixels within the outline thickness radius
	vec2 texel = 1.0 / screenSize;	// size of one texel in UV coordinates
	int radius = outlineThickness;		// outline radius in pixels

	// check the surrounding pixels in a square area of (2 * radius + 1) x ( 2 * radius + 1)
	bool edge = false; // flag to indicate if an edge pixel is found in the surrounding area
	for (int dy = -radius; dy <= radius && !edge; ++dy)
	{ // break the outer loop if an edge pixel is found
		for (int dx = -radius; dx <= radius; ++dx)
		{ // check each pixel in the square area
			vec2 offset = vec2(float(dx), float(dy)) * texel; // offset in UV coordinates
			if (sampleMask(TexCoords + offset) > 0.0) // sample the outline mask at the offset position
			{ // if any surrounding pixel is inside the object silhouette, mark as edge
				edge = true;
				break; // break the inner loop, since we only need one edge pixel
			}
		}
	}

	// output the outline color if an edge pixel was found, otherwise output the base color
	if (edge)
		FragColor = vec4(outlineColor, 1.0);
	else
		FragColor = baseColor;
}

vec3 grid(vec2 uv, int gridLineCount, float gridLineThickness, vec3 gridBgColor, vec3 gridLineColor)
{
	// convert UV coordinates to pixels using the screen size
	vec2 pixelCoords = uv * screenSize;

	// calculate the cell size in pixels (non-integer values are allowed)
	float cellWidth = screenSize.x / float(gridLineCount);
	float cellHeight = screenSize.y / float(gridLineCount);
	// calculate the position within the cell in pixels
	float cellPosX = mod(pixelCoords.x, cellWidth);
	float cellPosY = mod(pixelCoords.y, cellHeight);

	// compute the distance to the nearest vertical/horizontal grid line
	float distToVerticalLine = min(cellPosX, cellWidth - cellPosX);
	float distToHorizontalLine = min(cellPosY, cellHeight - cellPosY);
	float distToLine = min(distToVerticalLine, distToHorizontalLine);

	// ensure at least 0.5 pixel half-thickness (i.e. 1 pixel full thickness)
	float halfThickness = max(gridLineThickness * 0.5, 0.5);

	// anti-alias the grid lines based on the derivative of the pixel coordinates
	float aaSmoothness = 1.0;
	float mask = 1.0 - smoothstep(halfThickness - aaSmoothness, halfThickness + aaSmoothness, distToLine);

	// mix the background color and the line color based on the mask (ensure the mask is between 0 and 1)
	return mix(gridBgColor, gridLineColor, clamp(mask, 0.0, 1.0));
}

uint decodeId(vec3 c)
{
	uint r = uint(round(c.r * 255.0));
	uint g = uint(round(c.g * 255.0));
	uint b = uint(round(c.b * 255.0));
	return r | (g << 8) | (b << 16);
}

vec3 hashId(uint id)
{
	uint hash = id;
	hash ^= hash >> 16;
	hash *= 0x7feb352dU;
	hash ^= hash >> 15;
	hash *= 0x846ca68bU;
	hash ^= hash >> 16;
	uint r = (hash      ) & 0xFFu;
	uint g = (hash >>  8) & 0xFFu;
	uint b = (hash >> 16) & 0xFFu;
	return vec3(r, g, b) / 255.0;
}

vec3 boost(vec3 c)
{
	return pow(c, vec3(0.8)); // 0.8 gamma = brighten dark colors
}

float sampleMask(vec2 uv)
{
	return texture(outlineMaskTexture, uv).r;
}