/*
* PickingManager.cpp
* This file implements the PickingManager class, which handles object selection
* in the scene using color picking technique.
*/

#include "PickingManager.h"

#include <iostream>

// Constructor
// -----------
PickingManager::PickingManager() 
	: m_selectedObjectId{ 0 }, m_pickingWidth{ 0 }, m_pickingHeight{ 0 }
{
	m_pickingRenderPass = std::make_unique<RenderPass>();
}

// Destructor
// ----------
PickingManager::~PickingManager()
{
	Shutdown();
}

// Public Methods
// --------------
void PickingManager::Init(GLuint width, GLuint height)
{
	m_pickingWidth = width;
	m_pickingHeight = height;
	CreatePickingRenderPass();
	
	std::cout << "[INFO::PICKINGMANAGER::Init] Picking system initialized with dimensions " 
			  << width << "x" << height << std::endl;
}

void PickingManager::Resize(GLuint width, GLuint height)
{
	if (m_pickingWidth != width || m_pickingHeight != height)
	{
		m_pickingWidth = width;
		m_pickingHeight = height;
		CreatePickingRenderPass();
		
		std::cout << "[INFO::PICKINGMANAGER::Resize] Picking buffer resized to " 
				  << width << "x" << height << std::endl;
	}
}

std::uint32_t PickingManager::PickObject(GLint x, GLint y)
{
	if (!m_pickingRenderPass)
	{
		std::cerr << "[ERROR::PICKINGMANAGER::PickObject] Picking render pass not initialized" << std::endl;
		return 0;
	}
	
	// Bind the picking framebuffer for reading
	m_pickingRenderPass->SetAsReadBuffer();
	
	// Read the pixel at the specified coordinates
	GLubyte pixelData[3];
	glReadPixels(x, m_pickingHeight - y - 1, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
	
	// Convert the color back to object ID
	glm::vec3 color = glm::vec3(pixelData[0] / 255.0f, pixelData[1] / 255.0f, pixelData[2] / 255.0f);
	std::uint32_t objectId = ColorToObjectId(color);
	
	// Unbind the framebuffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	
	return objectId;
}

void PickingManager::SetSelectedObject(std::uint32_t objectId)
{
	if (m_selectedObjectId != objectId)
	{
		m_selectedObjectId = objectId;
		
		if (objectId != 0)
		{
			std::cout << "[INFO::PICKINGMANAGER::SetSelectedObject] Selected object with ID: " 
					  << objectId << std::endl;
		}
		else
		{
			std::cout << "[INFO::PICKINGMANAGER::SetSelectedObject] Selection cleared" << std::endl;
		}
	}
}

void PickingManager::ClearSelection()
{
	SetSelectedObject(0);
}

glm::vec3 PickingManager::ObjectIdToColor(std::uint32_t objectId)
{
	// Convert object ID to RGB color
	// We use 24-bit color space (8 bits per channel)
	// This allows for up to 16,777,216 unique objects
	
	GLubyte r = (objectId >> 16) & 0xFF; // red = high 8 bits
	GLubyte g = (objectId >> 8) & 0xFF;  // green = middle 8 bits
	GLubyte b = objectId & 0xFF;         // blue = low 8 bits
	
	return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
}

std::uint32_t PickingManager::ColorToObjectId(const glm::vec3& color)
{
	// Convert RGB color back to object ID
	GLubyte r = static_cast<GLubyte>(color.r * 255.0f + 0.5f);
	GLubyte g = static_cast<GLubyte>(color.g * 255.0f + 0.5f);
	GLubyte b = static_cast<GLubyte>(color.b * 255.0f + 0.5f);
	
	return (static_cast<std::uint32_t>(r) << 16) | 
		   (static_cast<std::uint32_t>(g) << 8) | 
		   static_cast<std::uint32_t>(b);
}

void PickingManager::Shutdown()
{
	if (m_pickingRenderPass)
	{
		m_pickingRenderPass->DeallocateResources();
		m_pickingRenderPass.reset();
	}
	
	m_selectedObjectId = 0;
	std::cout << "[INFO::PICKINGMANAGER::Shutdown] Picking system shut down" << std::endl;
}

// Private Methods
// ---------------
void PickingManager::CreatePickingRenderPass()
{
	if (!m_pickingRenderPass)
		return;
		
	// Configure the render pass specification for picking
	RenderPassSpecification pickingSpec;
	pickingSpec.Width = m_pickingWidth;
	pickingSpec.Height = m_pickingHeight;
	pickingSpec.ColorAttachmentCount = 1; // we only need one color attachment for picking
	pickingSpec.HasDepthAttachment = true; // we need depth testing for proper picking
	pickingSpec.HasStencilAttachment = false; // stencil not needed for picking
	
	// Create the picking render pass
	m_pickingRenderPass->Create(pickingSpec);
}