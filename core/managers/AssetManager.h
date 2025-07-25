/*
* AssetManager.h
* This file defines the AssetManager class, which is responsible for managing Asset objects in the engine.
* It provides methods to add, retrieve, and manage assets of various types
* and keeps track of the number of assets of each type.
* Also, when the AssetManager is destroyed, it deallocates resources for all assets.
*/

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <glad/glad.h> // holds OpenGL type definitions

#include "../Asset.h"
#include "../light/Light.h"
#include "../light/PointLight.h"
#include "../light/Spotlight.h"
#include "../light/DirectionalLight.h"
#include "../model/Model.h"
#include "../model/AssimpModel.h"
#include "../model/Shape.h"

class AssetManager
{
public:
	// Constructor
	// -----------
	AssetManager() = default;

	// Destructor
	// ----------
	~AssetManager();

	// Public Methods
	// --------------
	// Getters
	const std::vector<std::shared_ptr<Asset>>& GetAssets(AssetType assetType) const;
	const std::vector<std::shared_ptr<Asset>>& GetAssets(const std::string& assetType) const;
	const std::shared_ptr<Asset>& GetAssetByIndex(AssetType assetType, GLuint index) const;
	const std::shared_ptr<Asset>& GetAssetByIndex(const std::string& assetType, GLuint index) const;

	// Adds an asset to the asset manager
	void AddAsset(std::shared_ptr<Asset> asset);

	// Getters for the number of assets of different types
	const GLuint GetNAssets() const { return Asset::GetNAssets(); }
	const GLuint GetNLights() const { return Light::GetNLights(); }
	const GLuint GetNPointLights() const { return PointLight::GetNPointLights(); }
	const GLuint GetNSpotlights() const { return Spotlight::GetNSpotlights(); }
	const GLuint GetNDirectionalLights() const { return DirectionalLight::GetNDirectionalLights(); }
	const GLuint GetNModels() const { return Model::GetNModels(); }
	const GLuint GetNAssimpModels() const { return AssimpModel::GetNAssimpModels(); }
	const GLuint GetNShapes() const { return Shape::GetNShapes(); }

private:
	// Private Attributes
	// ------------------
	// Unordered map that holds vectors of assets, categorized by their type
	std::unordered_map<std::string, std::vector<std::shared_ptr<Asset>>> m_assets;

	// Private Methods
	// ---------------
	// Converts AssetType enum to string for map lookup
	std::string assetTypeToString(AssetType type) const;

};