/*
* Render_Pass.cpp
* This file implements the Render_Pass class, which is used to manage the lifecycle
* of a framebuffer object (FBO) for rendering to a texture.
*/

#include "Render_Pass.h"

// Constructor
// -----------
Render_Pass::Render_Pass() : fbo_id{ 0 }, rbo_id{ 0 }, depth_texture_id{ 0 } {}

// Destructor
// ----------
Render_Pass::~Render_Pass() { deallocate_resources(); }

// Public Methods
// --------------
void Render_Pass::create(const Render_Pass_Specification& spec)
{
	specification = spec;

	// deallocate resources if they have been already allocated
	if (fbo_id)
		deallocate_resources();

	// create a framebuffer object (FBO) and bind it
	glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

	// create color attachments using the specification
	if (specification.color_attachment_count > 0)
	{
		// resize the vector to hold the texture Ids for color attachments
		color_attachment_ids.resize(specification.color_attachment_count);
		// generate a texture for each color attachment
		glGenTextures(specification.color_attachment_count, color_attachment_ids.data());

		// bind each texture, set its parameters and attach it to the framebuffer
		for (int i = 0; i < specification.color_attachment_count; ++i)
		{
			glBindTexture(GL_TEXTURE_2D, color_attachment_ids[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, // prefer sized internal format for color attachments
						 specification.width, specification.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

			// set texture parameters for filtering and wrapping
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// clamp to edge to avoid sampling fringes
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// no mipmaps for render targets to avoid unnecessary overhead
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

			// attach the texture to the framebuffer as a color attachment with the appropriate index
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
								   color_attachment_ids[i], 0);
		}
	}

	// create depth and/or stencil attachment if specified
	if (specification.has_depth_attachment || specification.has_stencil_attachment)
	{
		if (specification.depth_as_texture && specification.has_depth_attachment)
		{
			// if depth is needed as a texture (for sampling in shaders later) and depth attachment 
			// is requested, allocate a depth (no stencil) or depth-stencil texture to sample from later

			// create the depth (or depth-stencil) texture and bind it
			glGenTextures(1, &depth_texture_id);
			glBindTexture(GL_TEXTURE_2D, depth_texture_id);

			// if stencil is also requested while needing depth as texture (rare here),
			// GL_DEPTH24_STENCIL8 could be used and GL_DEPTH_STENCIL_ATTACHMENT attached.
			// For now the approach is to keep it simple and ignore stencil when depth_as_texture = true.
			GLenum internal_format = GL_DEPTH_COMPONENT24; // prefer sized internal format for depth texture
			glTexImage2D(GL_TEXTURE_2D, 0, internal_format,
						 specification.width, specification.height, 0,
						 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

			// set texture parameters for filtering and wrapping
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			// attach the depth texture to the framebuffer
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture_id, 0);
		}
		else
		{
			// otherwise, allocate a renderbuffer object (RBO) for depth and/or stencil attachment 
			// (legacy, cannot be sampled)

			// create the renderbuffer object and bind it
			glGenRenderbuffers(1, &rbo_id);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo_id);

			// determine the appropriate format based on requested attachments
			GLenum format = GL_DEPTH24_STENCIL8;
			if (!specification.has_stencil_attachment)
				format = GL_DEPTH_COMPONENT24;
			else if (!specification.has_depth_attachment)
				format = GL_STENCIL_INDEX8;

			// allocate storage for the renderbuffer
			glRenderbufferStorage(GL_RENDERBUFFER, format,
								  specification.width, specification.height);

			// attach the appropiate renderbuffer to the framebuffer based on requested attachments
			if (specification.has_depth_attachment && specification.has_stencil_attachment)
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
										  GL_RENDERBUFFER, rbo_id);
			else if (specification.has_depth_attachment)
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
										  GL_RENDERBUFFER, rbo_id);
			else if (specification.has_stencil_attachment)
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
										  GL_RENDERBUFFER, rbo_id);
		}
	}

	// set the draw buffers based on the number of color attachments specified
	if (specification.color_attachment_count > 1)
	{ // if there are more than one color attachments
		std::vector<GLenum> attachments;
		for (int i{}; i < specification.color_attachment_count; ++i)
			attachments.push_back(GL_COLOR_ATTACHMENT0 + i);

		// set all attachments as draw buffers for multiple render targets (MRT)
		glDrawBuffers(specification.color_attachment_count, attachments.data());
		// explicitly set the read buffer to the first color attachment
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}
	else if (specification.color_attachment_count == 1)
	{ // if there is only one color attachment, set it as the draw and read buffer
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}
	else
	{ // if there are no color attachments, set the draw and read buffers to none
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	// check if the framebuffer is complete, if not, print an error message
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "[ERROR::RENDERPASS::create] Framebuffer is not complete!" << std::endl;

	// unbind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Render_Pass::set_as_draw_buffer(GLint index) const
{
	// bind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	if (index >= 0 && index < static_cast<GLint>(color_attachment_ids.size()))
	{ // if index is non-negative and within bounds
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + index); // set draw buffer to the specified color attachment
	}
	else
	{ // if index is negative (sentinel value) or out of bounds
		glDrawBuffer(GL_NONE); // set draw buffer to none
	}
}

void Render_Pass::set_as_read_buffer(GLint index) const
{
	// bind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	if (index >= 0 && index < static_cast<GLint>(color_attachment_ids.size()))
	{ // if index is non-negative and within bounds
		glReadBuffer(GL_COLOR_ATTACHMENT0 + index); // set read buffer to the specified color attachment
	}
	else
	{ // if index is negative (sentinel value) or out of bounds
		glReadBuffer(GL_NONE); // set read buffer to none
	}
}

void Render_Pass::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	// set the viewport to match the framebuffer size based on the specification
	glViewport(0, 0, specification.width, specification.height);
}

void Render_Pass::unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void Render_Pass::deallocate_resources()
{
	// ensure the framebuffer is unbound before deleting it
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// delete the framebuffer, color attachments, renderbuffer, and depth texture if they exist
	glDeleteFramebuffers(1, &fbo_id);
	if (!color_attachment_ids.empty())
	{
		glDeleteTextures(color_attachment_ids.size(), color_attachment_ids.data());
		color_attachment_ids.clear();
	}
	if (rbo_id)
		glDeleteRenderbuffers(1, &rbo_id);
	if (depth_texture_id)
		glDeleteTextures(1, &depth_texture_id);

	// reset the identifiers to zero after deallocation
	fbo_id = 0;
	rbo_id = 0;
	depth_texture_id = 0;
}

std::string Render_Pass::get_specification_str() const
{
	// return a string representation of the render pass specification
	return "{\n\tWidth: " + std::to_string(specification.width) + "\n" +
		"\tHeight: " + std::to_string(specification.height) + "\n" +
		"\tColor Attachment Count: " + std::to_string(specification.color_attachment_count) + "\n" +
		"\tHas Depth Attachment: " + (specification.has_depth_attachment ? "Yes" : "No") + "\n" +
		"\tHas Stencil Attachment: " + (specification.has_stencil_attachment ? "Yes" : "No") + "\n" +
		"\tDepth As Texture: " + (specification.depth_as_texture ? "Yes" : "No") + "\n}";
}

GLuint Render_Pass::get_texture_id(GLuint index) const
{
	// return the texture id for the specified index if it is within bounds
	if (index < color_attachment_ids.size())
		return color_attachment_ids[index];
	return 0;
}