/*
* SelectionManager.h
* Manages selection of scene assets through a color picking pass rendered on demand.
* It owns a dedicated offscreen RenderPass and a simple picking shader.
*/

#pragma once

#include <optional>
#include <unordered_map>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>

#include "../Asset.h"
#include "../renderer/RenderPass.h"

class Camera;
class Shader;
class AssetManager;

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

	// Get currently selected asset id (0 means none)
	std::uint32_t GetSelectedAssetId() const { return m_selectedAssetId; }

	// Returns a shared_ptr to the currently selected asset (may be null)
	std::shared_ptr<Asset> GetSelectedAsset(AssetManager* assetManager) const;

	// Clears the selection
	void ClearSelection() { m_selectedAssetId = 0; }

	// Deletes currently selected asset (and associated gizmo if a light)
	void DeleteSelected(AssetManager* assetManager);

	// Ensure internal picking FBO matches window size
	void Resize(GLuint width, GLuint height);

	// Sets picking shader (obtained after compilation)
	void SetPickingShader(const std::shared_ptr<Shader>& shader) { m_pickingShader = shader; }

private:
	// Private Attributes
	// ------------------
	RenderPass m_pickingPass;
	GLuint m_width, m_height;
	std::shared_ptr<Shader> m_pickingShader;
	std::optional<glm::ivec2> m_pendingPick; // screen coords (OpenGL origin bottom-left)

	std::uint32_t m_selectedAssetId;

	// Private Methods
	// ---------------
	// Ensures the picking pass is created
	void ensurePickingPass();

	// Reads the pixel id at the given coordinates from the picking FBO
	std::uint32_t readPixelId(GLint x, GLint y) const;

	// Finds an asset by its id among models and lights
	std::shared_ptr<Asset> findAssetById(AssetManager* assetManager, std::uint32_t id) const;

	// If the given asset is a gizmo model, resolves it to the owning light asset
	std::shared_ptr<Asset> resolveGizmoToLight(AssetManager* assetManager,
											   const std::shared_ptr<Asset>& gizmoModel) const;
};