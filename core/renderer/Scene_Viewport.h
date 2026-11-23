/*
 * Scene_Viewport.h
 * This file defines the Scene_Viewport struct, which is the rectangle of the window's framebuffer where the scene is drawn (for
 * example, the area that the windows of a GUI leave free). It is measured in pixels from the top-left corner of the framebuffer,
 * as the windowing system and the GUI measure their positions. It needs no OpenGL context, so its clipping is unit tested.
 */

#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations

struct Scene_Viewport
{
	// Public Attributes
	// -----------------
	GLint  x{ 0 };      // left edge, in pixels from the left of the framebuffer
	GLint  y{ 0 };      // top edge, in pixels from the top of the framebuffer
	GLuint width{ 0 };  // width in pixels
	GLuint height{ 0 }; // height in pixels

	// Public Methods
	// --------------
	// Returns this rectangle kept inside a framebuffer of the given size: the part of it that is inside the framebuffer, at least
	// one pixel wide and high (a rectangle that is empty or outside the framebuffer becomes one pixel at its nearest edge, since an
	// empty viewport has no aspect ratio). A framebuffer of width or height 0 (a minimized window) gives an empty rectangle
	Scene_Viewport clipped_to(GLuint framebuffer_width, GLuint framebuffer_height) const;

	// Returns true if the point (in pixels from the top-left corner of the framebuffer) is inside the rectangle (its left and top
	// edges are inside, its right and bottom edges are not)
	bool contains(GLdouble point_x, GLdouble point_y) const;

	// Returns the bottom edge of the rectangle in the coordinates of OpenGL, whose origin is the bottom-left corner of a framebuffer
	// of the given height (the y of glViewport)
	GLint get_opengl_y(GLuint framebuffer_height) const;

	// Operator overloads
	// ------------------
	bool operator==(const Scene_Viewport&) const = default;
};
