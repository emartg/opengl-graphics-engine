/*
* RenderPass.h
* This file defines the RenderPass class, which is used to manage the lifecycle
* of a framebuffer object (FBO) for rendering to a texture.
*/

#pragma once

#include <vector>
#include <iostream>

#include <glad/glad.h> // holds all OpenGL type declarations

// Forward declaration of the RenderPassSpecification struct
// to avoid circular dependency issues.
// This struct defines the specifications for the render pass
// such as width, height, number of color attachments, and depth/stencil attachments.
struct RenderPassSpecification
{
	GLuint Width{ 0 };
	GLuint Height{ 0 };
	GLuint ColorAttachmentCount{ 1 };
	GLboolean HasDepthAttachment{ true };
	GLboolean HasStencilAttachment{ true };
};

class RenderPass
{
public:
	// Constructors
	// ------------
	RenderPass();

	// Destructor
	// ----------
	~RenderPass();

	// Public Methods
	// --------------
	// Create a framebuffer object (FBO) with the specified render pass specification
	void Create(const RenderPassSpecification& spec);
	// Bind the framebuffer for rendering
	void Bind() const;
	// Unbind the framebuffer
	void Unbind() const;
	// Deallocate resources associated with the framebuffer
	void DeallocateResources();

	// Getters
	// -------
	// Get the texture ID for the specified color attachment index
	GLuint GetTextureId(GLuint index{ 0 }) const;

private:
	// Private Attributes
	// ------------------
	RenderPassSpecification m_specification;

	GLuint m_fboId; // framebuffer object identificator
	GLuint m_rboId; // renderbuffer object identificator for depth and stencil attachment

	std::vector<GLuint> m_colorAttachmentIds; // texture identificators for color attachments

};