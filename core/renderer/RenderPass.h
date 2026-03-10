/*
* Render_Pass.h
* This file defines the Render_Pass class, which is used to manage the lifecycle
* of a framebuffer object (FBO) for rendering to a texture.
*/

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations

// Struct that defines the specification for creating a render pass
struct Render_Pass_Specification
{
	GLuint width{ 0 };
	GLuint height{ 0 };
	GLuint color_attachment_count{ 1 };
	GLboolean has_depth_attachment{ true };
	GLboolean has_stencil_attachment{ true };
	// if true and has_depth_attachment is true, allocate a depth (or depth-stencil) texture instead of an RBO
	GLboolean depth_as_texture{ false };
};

class Render_Pass
{
public:
	// Constructors
	// ------------
	Render_Pass();

	// Destructor
	// ----------
	~Render_Pass();

	// Public Methods
	// --------------
	// create a framebuffer object (FBO) with the specified render pass specification
	void create(const Render_Pass_Specification& spec);
	// Set the framebuffer as the draw buffer for rendering
	void set_as_draw_buffer(GLint index = 0) const;
	// Set the framebuffer as the read buffer for reading back data
	void set_as_read_buffer(GLint index = 0) const;
	// bind the framebuffer for rendering
	void bind() const;
	// unbind the framebuffer
	void unbind() const;
	// Deallocate resources associated with the framebuffer
	void deallocate_resources();

	// Getters
	// Get the render pass specification as a string
	std::string get_specification_str() const;
	// Get the framebuffer object id
	GLuint get_fbo_id() const { return fbo_id; }
	// Get the renderbuffer object id
	GLuint get_rbo_id() const { return rbo_id; }
	// Get the texture id for the specified color attachment index
	GLuint get_texture_id(GLuint index = 0) const;
	// Get the texture id of the depth attachment if it was allocated as a texture (0 if not)
	GLuint get_depth_texture_id() const { return depth_texture_id; }

private:
	// Private Attributes
	// ------------------
	Render_Pass_Specification specification;

	GLuint fbo_id; // framebuffer object identificator
	GLuint rbo_id; // renderbuffer object identificator for depth and stencil attachment
	GLuint depth_texture_id; // depth (or depth-stencil) attachment texture id if depth_as_texture is true

	std::vector<GLuint> color_attachment_ids; // texture identificators for color attachments

};