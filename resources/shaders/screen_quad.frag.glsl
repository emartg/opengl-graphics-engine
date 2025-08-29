#version 420 core
out vec4 FragColor;

// struct to hold screen debug mode parameters
struct ScreenDebugParams
{
	int debugMode;				// current debug mode (0 = normal, 1 = solid color, 2 = grid overlay, 
								//					   3 = inverted colors, 4 = picking colors)
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
uniform sampler2D screenTexture;

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

void main()
{
	if (screenDebugParams.debugMode == 0)
	{ // normal rendering: sample the screen texture
		FragColor = texture(screenTexture, TexCoords);
	}
	else if (screenDebugParams.debugMode == 1)
	{ // solid color rendering: use the solid color specified in the debug mode parameters
		FragColor = vec4(screenDebugParams.solidColor, 1.0);
	}
	else if (screenDebugParams.debugMode == 2)
	{ // grid overlay rendering: create a grid overlay on the UV coordinates with the specified parameters
		int count = max(screenDebugParams.gridLineCount, 1); // ensure at least 1 line
		float thickness = max(screenDebugParams.gridLineThickness, 1.0f); // ensure at least 1 px thickness
		// compute the grid color based on the texture coords as UV coords and other parameters
		vec3 color = grid(TexCoords, count, thickness, 
						  screenDebugParams.gridBgColor, screenDebugParams.gridLineColor);
		FragColor = vec4(color, 1.0);
	}
	else if (screenDebugParams.debugMode == 3)
	{ // inverted color rendering: sample the screen texture and invert the color
		vec4 color = texture(screenTexture, TexCoords);
		FragColor = vec4(1.0 - color.rgb, 1.0);
	}
	else if (screenDebugParams.debugMode == 4)
	{ // picking color rendering: sample the screen texture and decode the id to a unique color per object
		vec3 raw = texture(screenTexture, TexCoords).rgb; // raw encoded id color from picking pass
		uint id = decodeId(raw); // decode the id (0 = no object)
		if (id == 0u)
		{ 
			// if no object, render black
			FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		}
		else
		{ 
			// if object, render a boosted color based on the hashed id to get a unique but consistent
			// color for each object
			vec3 hashed = boost(hashId(id)); // boost to make dark colors more visible
			FragColor = vec4(hashed, 1.0);
		}
	}
	else
	{ // unrecognized debugMode: default to black solid color
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
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