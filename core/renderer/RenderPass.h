/*
* RenderPass.h
* This file defines the RenderPass class, which is used to manage the lifecycle
* of a framebuffer object (FBO) for rendering to a texture.
*/

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations

// this struct defines the specifications for the render pass
struct RenderPassSpecification
{
	GLuint Width{ 0 };
	GLuint Height{ 0 };
	GLuint ColorAttachmentCount{ 1 };
	GLboolean HasDepthAttachment{ true };
	GLboolean HasStencilAttachment{ true };
	// if true and HasDepthAttachment is true, allocate a depth (or depth-stencil) texture instead of an RBO
	GLboolean DepthAsTexture{ false };
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
	// Set the framebuffer as the draw buffer for rendering
	void SetAsDrawBuffer(GLint index = 0) const;
	// Set the framebuffer as the read buffer for reading back data
	void SetAsReadBuffer(GLint index = 0) const;
	// Bind the framebuffer for rendering
	void Bind() const;
	// Unbind the framebuffer
	void Unbind() const;
	// Deallocate resources associated with the framebuffer
	void DeallocateResources();

	// Getters
	// Get the render pass specification as a string
	std::string GetSpecificationStr() const;
	// Get the framebuffer object ID
	GLuint GetFboId() const { return m_fboId; }
	// Get the renderbuffer object ID
	GLuint GetRboId() const { return m_rboId; }
	// Get the texture ID for the specified color attachment index
	GLuint GetTextureId(GLuint index = 0) const;
	// Get the texture ID of the depth attachment if it was allocated as a texture (0 if not)
	GLuint GetDepthTextureId() const { return m_depthTextureId; }

private:
	// Private Attributes
	// ------------------
	RenderPassSpecification m_specification;

	GLuint m_fboId; // framebuffer object identificator
	GLuint m_rboId; // renderbuffer object identificator for depth and stencil attachment
	GLuint m_depthTextureId; // depth (or depth-stencil) attachment texture id if DepthAsTexture is true

	std::vector<GLuint> m_colorAttachmentIds; // texture identificators for color attachments

};