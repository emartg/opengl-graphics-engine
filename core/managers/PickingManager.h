/*
* PickingManager.h
* This file defines the PickingManager class, which is responsible for handling object selection
* in the scene using color picking technique. It manages:
* - Selected object state
* - Color picking render pass
* - Mouse click to object ID conversion
* - Selection change notifications
*/

#pragma once

#include <memory>
#include <cstdint>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../Asset.h"
#include "../renderer/RenderPass.h"

class PickingManager
{
public:
	// Constructor
	// -----------
	PickingManager();

	// Destructor
	// ----------
	~PickingManager();

	// Public Methods
	// --------------
	// Initializes the picking system with the specified dimensions
	void Init(GLuint width, GLuint height);
	
	// Updates the picking buffer size when window is resized
	void Resize(GLuint width, GLuint height);
	
	// Performs object picking at the specified screen coordinates
	// Returns the ID of the picked object, or 0 if no object was picked
	std::uint32_t PickObject(GLint x, GLint y);
	
	// Sets the currently selected object by ID
	void SetSelectedObject(std::uint32_t objectId);
	
	// Clears the current selection
	void ClearSelection();
	
	// Checks if an object is currently selected
	bool HasSelection() const { return m_selectedObjectId != 0; }
	
	// Gets the ID of the currently selected object
	std::uint32_t GetSelectedObjectId() const { return m_selectedObjectId; }
	
	// Converts an object ID to a unique RGB color for picking
	static glm::vec3 ObjectIdToColor(std::uint32_t objectId);
	
	// Converts an RGB color back to an object ID
	static std::uint32_t ColorToObjectId(const glm::vec3& color);
	
	// Gets the picking render pass for rendering with unique colors
	RenderPass* GetPickingRenderPass() { return m_pickingRenderPass.get(); }
	
	// Cleanup resources
	void Shutdown();

private:
	// Private Attributes
	// ------------------
	std::uint32_t m_selectedObjectId; // ID of the currently selected object (0 = no selection)
	
	std::unique_ptr<RenderPass> m_pickingRenderPass; // FBO for color picking
	GLuint m_pickingWidth, m_pickingHeight; // dimensions of the picking buffer
	
	// Private Methods
	// ---------------
	// Creates or recreates the picking render pass with current dimensions
	void CreatePickingRenderPass();
};