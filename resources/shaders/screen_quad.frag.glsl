#version 420 core
out vec4 FragColor;

struct DebugModeParams
{
	int debugMode;		// debug mode (0: normal, 1: solid color, 2: inverted color, 3: grid overlay)
	vec3 solidColor;	// color to use when the debug mode is 1
	int nLines;			// number of lines in the grid overlay when the debug mode is 3
	float lineWidth;	// width of the lines in the grid overlay when the debug mode is 3
	float bgColor;		// background color for the grid overlay when the debug mode is 3
	float fgColor;		// foreground color for the grid overlay when the debug mode is 3 (line color)
};

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform DebugModeParams debugModeParams;

// Function to create a grid overlay on the UV coordinates
// Parameters:
// - uv: the UV coordinates of the fragment
// - lines: number of lines in the grid
// - lineWidth: width of the lines in the grid
// - bgColor: color for the background of the grid
// - fgColor: color for the grid lines
vec3 grid(vec2 uv, int nLines, float lineWidth, float bgColor, float fgColor);

void main()
{
	if (debugModeParams.debugMode == 0)
	{ // normal rendering: sample the screen texture
		FragColor = texture(screenTexture, TexCoords);
	}
	else if (debugModeParams.debugMode == 1)
	{ // solid color rendering: use the solid color specified in the debug mode parameters
		FragColor = vec4(debugModeParams.solidColor, 1.0);
	}
	else if (debugModeParams.debugMode == 2)
	{ // inverted color rendering: sample the screen texture and invert the color
		vec4 color = texture(screenTexture, TexCoords);
		FragColor = vec4(1.0 - color.rgb, 1.0);
	}
	else if (debugModeParams.debugMode == 3)
	{ // grid overlay rendering: create a grid overlay on the UV coordinates with the specified parameters
		vec2 uv = TexCoords; // use the texture coordinates as UV coordinates
		// create the grid color based on the UV coordinates and the grid parameters
		vec3 gridColor = grid(uv, debugModeParams.nLines, debugModeParams.lineWidth, 
								  debugModeParams.bgColor, debugModeParams.fgColor);
		FragColor = vec4(gridColor, 1.0); // set the fragment color to the grid color with full opacity
	}
	else
	{ // unrecognized debugMode: default to black
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}

vec3 grid(vec2 uv, int nLines, float lineWidth, float bgColor, float fgColor)
{
	vec2 g = fract(uv * nLines); // get the fractional part of the UV coords scaled by the number of lines

	// use step to create a grid pattern by checking if the fractional part is less than the line width,
	// so that 'line' is 1.0 if we are within the line width and 0.0 otherwise
	float line = step(g.x, lineWidth) + step(g.y, lineWidth);
	// use clamp to ensure the line value is between 0.0 and 1.0,
	// so that 'mask' is 1.0 if we are within the line width and 0.0 otherwise
	float mask = clamp(line, 0.0, 1.0); // clamp the line value to be between 0 and 1

	// mix the background and foreground colors based on the mask and return the resulting color
	return mix(vec3(bgColor), vec3(fgColor), mask);
}