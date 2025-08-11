/*
* AssetManager.cpp
* This file implements the AssetManager class, which is responsible for managing Asset objects in the engine.
* It provides methods to add, retrieve, and manage assets of various types
* and keeps track of the number of assets of each type.
* Also, when the AssetManager is destroyed, it deallocates resources for all assets.
*/

#include "AssetManager.h"

// Destructor
// ----------
AssetManager::~AssetManager()
{
	// for each asset type in the map, iterate through the vector of assets and deallocate resources
	for (auto& [assetType, assets] : m_assets)
		for (auto& asset : assets)
			if (asset) // check if the asset is not null
				asset->DeallocateResources(); // deallocate resources for the asset
}

// Public Methods
// --------------
const std::vector<std::shared_ptr<Asset>>& AssetManager::GetAssets(AssetType assetType) const
{
	std::string assetTypeStr = assetTypeToString(assetType); // convert AssetType to str for map lookup

	return GetAssets(assetTypeStr); // delegate to the string-based GetAssets method
}

const std::vector<std::shared_ptr<Asset>>& AssetManager::GetAssets(const std::string& assetType) const
{
	// find the asset type in the map
	auto it = m_assets.find(assetType);

	// ensure at least one asset of the type exists
	if (it != m_assets.end())
		return it->second; // return the vector of assets of the specified type

	// return an empty vector if the asset type is not found
	static const std::vector<std::shared_ptr<Asset>> empty;
	return empty;
}

const std::shared_ptr<Asset>& AssetManager::GetAssetByIndex(AssetType assetType, GLuint index) const
{
	std::string assetTypeStr = assetTypeToString(assetType); // convert AssetType to str for map lookup

	return GetAssetByIndex(assetTypeStr, index); // delegate to the string-based GetAssetByIndex method
}

const std::shared_ptr<Asset>& AssetManager::GetAssetByIndex(const std::string& assetType, GLuint index) const
{
	// find the asset type in the map
	auto it = m_assets.find(assetType);

	// ensure at least one asset of the type exists and index is within bounds
	if (it != m_assets.end() && index < it->second.size())
		return it->second[index]; // return the asset of the specified type at the specified index

	// return an empty shared_ptr if the asset type is not found or index is out of bounds
	static const std::shared_ptr<Asset> empty;
	return empty;
}

void AssetManager::AddAsset(std::shared_ptr<Asset> asset)
{
	// convert AssetType to string for map lookup
	std::string assetType = assetTypeToString(asset->GetType());

	// add asset to the corresponding vector in the map
	m_assets[assetType].emplace_back(asset);

	// print the type, id, and name of the added asset
	std::cout << "[INFO::ASSETMANAGER::AddAsset] Added "
		<< assetType << " asset with ID " << asset->GetId() << " and name " << asset->GetName() << std::endl;
}

void AssetManager::RemoveAssetById(std::uint32_t id)
{
	for (auto& [assetType, assets] : m_assets)
	{ // iterate through each asset of each type in the map
		// try to find the asset with the specified ID in the vector of assets of the current type
		auto it = std::remove_if(assets.begin(), assets.end(),
								 [id](const std::shared_ptr<Asset>& asset) { return asset->GetId() == id; });

		if (it != assets.end())
		{ // if an asset with the ID was found, print a message and erase it
			std::cout << "[INFO::ASSETMANAGER::RemoveAssetById] Removed "
				<< assetType << " asset with ID: " << id << std::endl;
			assets.erase(it, assets.end());
			return; // exit after removing the asset
		}
	}

	// if no asset with the specified ID was found, print an error message
	std::cerr << "[ERROR::ASSETMANAGER::RemoveAssetById] No asset found with ID: " << id << std::endl;
}

// Private Methods
// ---------------
std::string AssetManager::assetTypeToString(AssetType type) const
{
	std::string assetTypeStr{};
	switch (type)
	{
		case AssetType::CAMERA:
			assetTypeStr = "CAMERA";
			break;
		case AssetType::LIGHT:
			assetTypeStr = "LIGHT";
			break;
		case AssetType::MODEL:
			assetTypeStr = "MODEL";
			break;
		case AssetType::SHADER:
			assetTypeStr = "SHADER";
			break;
		case AssetType::TEXTURE:
			assetTypeStr = "TEXTURE";
			break;
		default:
			std::cerr << "AssetType not defined!" << std::endl;
			throw std::invalid_argument("Invalid asset type");
	}

	return assetTypeStr;
}