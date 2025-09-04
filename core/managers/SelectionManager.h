/*
* SelectionManager.h
* Manages selection of assets in the scene via picking passes,
* and an outlining mask pass for the selected asset.
*/

#pragma once

#include <optional>
#include <memory>
#include <cstdint>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

#include "../Asset.h"
#include "../renderer/RenderPass.h"

class Camera;
class Shader;
class AssetManager;

// Struct to hold outline parameters for the selected asset
struct OutlineParams
{
	glm::vec3 color{ 0.0f, 0.95f, 1.0f };	// outline color - highly visible cyan by default
	GLuint thickness{ 2 };					// outline thickness in pixels - 2 by default (must be >= 1)
};

class SelectionManager
{
public:
	// Constructor
	// -----------
	SelectionManager();

	// Destructor
	// ----------
	~SelectionManager();

	// Public Methods
	// --------------
	// Queues a pick request at the given window coordinates (origin top-left from GLFW)
	void QueuePick(GLdouble mouseX, GLdouble mouseY, GLsizei windowWidth, GLsizei windowHeight);

	// Called from the renderer at frame start to perform the queued pick (if any)
	void ProcessPendingPick(const Camera* camera, AssetManager* assetManager);

	// Renders (or re-renders) the full picking buffer for visualization (no selection readback)
	void RenderPickingVisualization(const Camera* camera, AssetManager* assetManager);

	// Get currently selected asset id (0 means none)
	std::uint32_t GetSelectedAssetId() const { return m_selectedAssetId; }

	// Returns a shared_ptr to the currently selected asset (may be null)
	std::shared_ptr<Asset> GetSelectedAsset(AssetManager* assetManager) const;

	// Clears the selection and explicitly clears the outline mask
	void ClearSelection();

	// Deletes currently selected asset (and associated gizmo if a light)
	void DeleteSelected(AssetManager* assetManager);

	// Ensure internal picking FBO matches window size
	void Resize(GLuint width, GLuint height);

	// Sets picking shader (obtained after compilation)
	void SetPickingShader(const std::shared_ptr<Shader>& shader) { m_pickingShader = shader; }

	// Returns the texture id of the picking color attachment (0 if unavailable or FBO not created)
	GLuint GetPickingTextureId() const;

	// Renders the outline mask for the selected asset (if any)
	void RenderOutlineMask(const Camera* camera, AssetManager* assetManager);

	// Returns the texture id of the outline mask color attachment (0 if unavailable or FBO not created)
	GLuint GetOutlineMaskTextureId() const;

	// Returns the outline parameters
	const OutlineParams& GetOutlineParams() const { return m_outlineParams; }

	// Sets the outline parameters
	void SetOutlineParams(const OutlineParams& params) { m_outlineParams = params; }

private:
	// Private Attributes
	// ------------------
	// general selection manager attributes
	GLuint m_width, m_height;
	std::uint32_t m_selectedAssetId;

	// picking attributes
	RenderPass m_pickingPass;
	std::shared_ptr<Shader> m_pickingShader;
	std::optional<glm::ivec2> m_pendingPick; // screen coords (OpenGL origin bottom-left)

	// outline attributes
	RenderPass m_outlinePass; // FBO for rendering the outline mask (single channel via RGBA8)
	OutlineParams m_outlineParams; // parameters for the outline effect

	// Private Methods
	// ---------------
	// Ensures the picking pass is created
	void ensurePickingPass();

	// Ensures the outline pass is created
	void ensureOutlinePass();

	// Clears the outline mask FBO (used when selection changes to a non-outline-eligible asset or is cleared)
	void clearOutlineMask();

	// Returns true if the asset should have an outline mask generated (real scene model, not a gizmo)
	bool isOutlineEligible(const std::shared_ptr<Asset>& asset) const;

	// Reads the pixel id at the given coordinates from the picking FBO
	std::uint32_t readPixelId(GLint x, GLint y) const;

	// Finds an asset by its id among models and lights
	std::shared_ptr<Asset> findAssetById(AssetManager* assetManager, std::uint32_t id) const;

	// If the given asset is a gizmo model, resolves it to the owning light asset
	std::shared_ptr<Asset> resolveGizmoToLight(AssetManager* assetManager,
											   const std::shared_ptr<Asset>& gizmoModel) const;

};