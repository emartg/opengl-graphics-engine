/*
 * Scene_Viewport.cpp
 * This file implements the Scene_Viewport struct, which is the rectangle of the window's framebuffer where the scene is drawn.
 */

#include "Scene_Viewport.h"

#include <algorithm>
#include <cstdint>

namespace
{
	// Keeps the interval from start to start plus size inside the interval from 0 to the limit (the limit is at least 1), at least
	// one pixel long, and stores its start and its size
	void clip_interval(std::int64_t start, std::int64_t size, std::int64_t limit, GLint& clipped_start, GLuint& clipped_size)
	{
		const std::int64_t first = std::clamp<std::int64_t>(start, 0, limit - 1);
		const std::int64_t last  = std::clamp<std::int64_t>(start + size, first + 1, limit);
		clipped_start            = static_cast<GLint>(first);
		clipped_size             = static_cast<GLuint>(last - first);
	}
} // namespace

// Public Methods
// --------------
Scene_Viewport Scene_Viewport::clipped_to(GLuint framebuffer_width, GLuint framebuffer_height) const
{
	if (framebuffer_width == 0 || framebuffer_height == 0)
		return Scene_Viewport{}; // nothing is rendered into an empty framebuffer

	Scene_Viewport clipped;
	clip_interval(x, width, framebuffer_width, clipped.x, clipped.width);
	clip_interval(y, height, framebuffer_height, clipped.y, clipped.height);
	return clipped;
}

bool Scene_Viewport::contains(GLdouble point_x, GLdouble point_y) const
{
	return point_x >= x && point_x < static_cast<GLdouble>(x) + width && point_y >= y && point_y < static_cast<GLdouble>(y) + height;
}

GLint Scene_Viewport::get_opengl_y(GLuint framebuffer_height) const
{
	return static_cast<GLint>(framebuffer_height) - y - static_cast<GLint>(height);
}
