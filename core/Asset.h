/*
* Asset.h
* This file defines the Asset class, which is an abstract base class
* to manage an asset meant to be used by the engine.
*/

#pragma once

#include <string>
#include <cstdint>

enum class AssetType { UNDEFINED = 0, CAMERA, LIGHT, MODEL, SHADER, TEXTURE };

class Asset
{
public:
	// Constructors
	// ------------
	// This constructor initializes the asset with a unique id, a name, and an asset type.
	// The id is based on a static counter that increments each time a new asset is created.
	// It is pre-incremented to ensure the first asset has id 1, not 0 (since 0 is reserved
	// as a sentinel value for undefined assets, e.g., when no assets are selected)
	Asset(const std::string& name, const AssetType type)
		: id{ ++assetsCount }, name{ name }, type{ type } { nAssets++; }

	// Virtual destructor
	// ------------------
	virtual ~Asset() { nAssets--; }

	// Public Functions
	// ----------------
	// Loads the asset
	virtual void Load() = 0;

	// Deallocates all the resources of the asset
	virtual void DeallocateResources() = 0;

	// Getters
	virtual std::uint32_t GetId() const { return id; }
	virtual const std::string& GetName() const { return name; }
	virtual const AssetType& GetType() const { return type; }

	// Setters
	virtual void SetId(std::uint32_t id) { this->id = id; }
	virtual void SetName(const std::string& name) { this->name = name; }
	virtual void SetType(const AssetType& type) { this->type = type; }

	// Static Public Functions
	// -----------------------
	static std::uint32_t GetAssetsCount() { return assetsCount; }
	static std::uint32_t GetNAssets() { return nAssets; }

private:
	// Static Private Attributes
	// -------------------------
	static std::uint32_t assetsCount; // total number of assets created
	static std::uint32_t nAssets; // current number of assets in the scene

protected:
	// Protected Attributes
	// --------------------
	std::uint32_t id; // unique identifier for the asset
	std::string name;
	AssetType type;

};