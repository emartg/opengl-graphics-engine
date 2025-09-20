/*
* RenderPass.cpp
* This file implements the RenderPass class, which is used to manage the lifecycle
* of a framebuffer object (FBO) for rendering to a texture.
*/

#include "RenderPass.h"

// Constructor
// -----------
RenderPass::RenderPass() : m_fboId{ 0 }, m_rboId{ 0 }, m_depthTextureId{ 0 } {}

// Destructor
// ----------
RenderPass::~RenderPass() { DeallocateResources(); }

// Public Methods
// --------------
void RenderPass::Create(const RenderPassSpecification& spec)
{
	m_specification = spec;

	// deallocate resources if they have been already allocated
	if (m_fboId)
		DeallocateResources();

	// create a framebuffer object (FBO) and bind it
	glGenFramebuffers(1, &m_fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

	// create color attachments using the specification
	if (m_specification.ColorAttachmentCount > 0)
	{
		// resize the vector to hold the texture Ids for color attachments
		m_colorAttachmentIds.resize(m_specification.ColorAttachmentCount);
		// generate a texture for each color attachment
		glGenTextures(m_specification.ColorAttachmentCount, m_colorAttachmentIds.data());

		// bind each texture, set its parameters and attach it to the framebuffer
		for (int i = 0; i < m_specification.ColorAttachmentCount; ++i)
		{
			glBindTexture(GL_TEXTURE_2D, m_colorAttachmentIds[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, // prefer sized internal format for color attachments
						 m_specification.Width, m_specification.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

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
								   m_colorAttachmentIds[i], 0);
		}
	}

	// create depth and/or stencil attachment if specified
	if (m_specification.HasDepthAttachment || m_specification.HasStencilAttachment)
	{
		if (m_specification.DepthAsTexture && m_specification.HasDepthAttachment)
		{
			// if depth is needed as a texture (for sampling in shaders later) and depth attachment 
			// is requested, allocate a depth (no stencil) or depth-stencil texture to sample from later

			// create the depth (or depth-stencil) texture and bind it
			glGenTextures(1, &m_depthTextureId);
			glBindTexture(GL_TEXTURE_2D, m_depthTextureId);

			// if stencil is also requested while needing depth as texture (rare here),
			// GL_DEPTH24_STENCIL8 could be used and GL_DEPTH_STENCIL_ATTACHMENT attached.
			// For now the approach is to keep it simple and ignore stencil when DepthAsTexture = true.
			GLenum internalFmt = GL_DEPTH_COMPONENT24; // prefer sized internal format for depth texture
			glTexImage2D(GL_TEXTURE_2D, 0, internalFmt,
						 m_specification.Width, m_specification.Height, 0,
						 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

			// set texture parameters for filtering and wrapping
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			// attach the depth texture to the framebuffer
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTextureId, 0);
		}
		else
		{
			// otherwise, allocate a renderbuffer object (RBO) for depth and/or stencil attachment 
			// (legacy, cannot be sampled)

			// create the renderbuffer object and bind it
			glGenRenderbuffers(1, &m_rboId);
			glBindRenderbuffer(GL_RENDERBUFFER, m_rboId);

			// determine the appropriate format based on requested attachments
			GLenum format = GL_DEPTH24_STENCIL8;
			if (!m_specification.HasStencilAttachment)
				format = GL_DEPTH_COMPONENT24;
			else if (!m_specification.HasDepthAttachment)
				format = GL_STENCIL_INDEX8;

			// allocate storage for the renderbuffer
			glRenderbufferStorage(GL_RENDERBUFFER, format,
								  m_specification.Width, m_specification.Height);

			// attach the appropiate renderbuffer to the framebuffer based on requested attachments
			if (m_specification.HasDepthAttachment && m_specification.HasStencilAttachment)
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
										  GL_RENDERBUFFER, m_rboId);
			else if (m_specification.HasDepthAttachment)
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
										  GL_RENDERBUFFER, m_rboId);
			else if (m_specification.HasStencilAttachment)
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
										  GL_RENDERBUFFER, m_rboId);
		}
	}

	// set the draw buffers based on the number of color attachments specified
	if (m_specification.ColorAttachmentCount > 1)
	{ // if there are more than one color attachments
		std::vector<GLenum> attachments;
		for (int i{}; i < m_specification.ColorAttachmentCount; ++i)
			attachments.push_back(GL_COLOR_ATTACHMENT0 + i);

		// set all attachments as draw buffers for multiple render targets (MRT)
		glDrawBuffers(m_specification.ColorAttachmentCount, attachments.data());
		// explicitly set the read buffer to the first color attachment
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}
	else if (m_specification.ColorAttachmentCount == 1)
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
		std::cerr << "[ERROR::RENDERPASS::Create] Framebuffer is not complete!" << std::endl;

	// unbind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPass::SetAsDrawBuffer(GLint index) const
{
	// bind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
	if (index >= 0 && index < static_cast<GLint>(m_colorAttachmentIds.size()))
	{ // if index is non-negative and within bounds
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + index); // set draw buffer to the specified color attachment
	}
	else
	{ // if index is negative (sentinel value) or out of bounds
		glDrawBuffer(GL_NONE); // set draw buffer to none
	}
}

void RenderPass::SetAsReadBuffer(GLint index) const
{
	// bind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
	if (index >= 0 && index < static_cast<GLint>(m_colorAttachmentIds.size()))
	{ // if index is non-negative and within bounds
		glReadBuffer(GL_COLOR_ATTACHMENT0 + index); // set read buffer to the specified color attachment
	}
	else
	{ // if index is negative (sentinel value) or out of bounds
		glReadBuffer(GL_NONE); // set read buffer to none
	}
}

void RenderPass::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
	// set the viewport to match the framebuffer size based on the specification
	glViewport(0, 0, m_specification.Width, m_specification.Height);
}

void RenderPass::Unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void RenderPass::DeallocateResources()
{
	// ensure the framebuffer is unbound before deleting it
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// delete the framebuffer, color attachments, renderbuffer, and depth texture if they exist
	glDeleteFramebuffers(1, &m_fboId);
	if (!m_colorAttachmentIds.empty())
	{
		glDeleteTextures(m_colorAttachmentIds.size(), m_colorAttachmentIds.data());
		m_colorAttachmentIds.clear();
	}
	if (m_rboId)
		glDeleteRenderbuffers(1, &m_rboId);
	if (m_depthTextureId)
		glDeleteTextures(1, &m_depthTextureId);

	// reset the identifiers to zero after deallocation
	m_fboId = 0;
	m_rboId = 0;
	m_depthTextureId = 0;
}

std::string RenderPass::GetSpecificationStr() const
{
	// return a string representation of the render pass specification
	return "{\n\tWidth: " + std::to_string(m_specification.Width) + "\n" +
		"\tHeight: " + std::to_string(m_specification.Height) + "\n" +
		"\tColor Attachment Count: " + std::to_string(m_specification.ColorAttachmentCount) + "\n" +
		"\tHas Depth Attachment: " + (m_specification.HasDepthAttachment ? "Yes" : "No") + "\n" +
		"\tHas Stencil Attachment: " + (m_specification.HasStencilAttachment ? "Yes" : "No") +
		"\tDepth As Texture: " + (m_specification.DepthAsTexture ? "Yes" : "No")
		+ "\n}";
}

GLuint RenderPass::GetTextureId(GLuint index) const
{
	// return the texture ID for the specified index if it is within bounds
	if (index < m_colorAttachmentIds.size())
		return m_colorAttachmentIds[index];
	return 0;
}