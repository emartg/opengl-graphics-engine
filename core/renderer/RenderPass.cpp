/*
* RenderPass.cpp
* This file implements the RenderPass class, which is used to manage the lifecycle
* of a framebuffer object (FBO) for rendering to a texture.
*/

#include "RenderPass.h"

// Constructor
// -----------
RenderPass::RenderPass() : m_fboId{ 0 }, m_rboId{ 0 } {}

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

		// bind each texture and set its parameters
		for (int i = 0; i < m_specification.ColorAttachmentCount; ++i)
		{
			glBindTexture(GL_TEXTURE_2D, m_colorAttachmentIds[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_specification.Width, m_specification.Height,
						 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
								   m_colorAttachmentIds[i], 0);
		}
	}

	// create a renderbuffer object (RBO) for depth and stencil attachment if specified
	if (m_specification.HasDepthAttachment || m_specification.HasStencilAttachment)
	{
		// generate a renderbuffer object and bind it
		glGenRenderbuffers(1, &m_rboId);
		glBindRenderbuffer(GL_RENDERBUFFER, m_rboId);

		// set the renderbuffer storage format based on the specification
		GLenum format = GL_DEPTH24_STENCIL8; // default format for depth and stencil attachment	
		if (!m_specification.HasStencilAttachment)
			format = GL_DEPTH_COMPONENT24; // use only depth attachment
		else if (!m_specification.HasDepthAttachment)
			format = GL_STENCIL_INDEX8; // use only stencil attachment

		// allocate storage for the renderbuffer and attach it to the framebuffer
		glRenderbufferStorage(GL_RENDERBUFFER, format, m_specification.Width, m_specification.Height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rboId);
	}

	// set the draw buffers based on the number of color attachments specified
	if (m_specification.ColorAttachmentCount > 1)
	{ // if there are more than one color attachments
		std::vector<GLenum> attachments;
		for (int i = 0; i < m_specification.ColorAttachmentCount; ++i)
			attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
		glDrawBuffers(m_specification.ColorAttachmentCount, attachments.data());
	}
	else if (m_specification.ColorAttachmentCount == 1)
	{ // if there is only one color attachment, set it as the draw and read buffer
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}
	else if (m_specification.ColorAttachmentCount == 0)
	{ // if there are no color attachments, set the draw and read buffers to none
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	// check if the framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR::FRAMEBUFFER::Framebuffer is not complete!" << std::endl;

	// unbind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	// delete the framebuffer, color attachments, and renderbuffer if they exist
	glDeleteFramebuffers(1, &m_fboId);
	if (!m_colorAttachmentIds.empty())
	{
		glDeleteTextures(m_colorAttachmentIds.size(), m_colorAttachmentIds.data());
		m_colorAttachmentIds.clear();
	}
	if (m_rboId)
		glDeleteRenderbuffers(1, &m_rboId);

	// reset the identifiers to zero after deallocation
	m_fboId = 0;
	m_rboId = 0;
}

GLuint RenderPass::GetTextureId(GLuint index) const
{
	// return the texture ID for the specified index if it is within bounds
	if (index < m_colorAttachmentIds.size())
		return m_colorAttachmentIds[index];
	return 0;
}