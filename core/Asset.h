/*
* Asset.h
* This file defines the Asset class, which is an abstract base class
* to manage an asset meant to be used by the engine.
*/

#pragma once

#include <string>

enum class AssetType { UNDEFINED = 0, CAMERA, LIGHT, MODEL, SHADER, TEXTURE };

class Asset
{
public:
	// Constructors
	// ------------
	Asset(const std::string& name, const AssetType type)
		: id{ nAssets++ }, name{ name }, type{ type } {}

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
	virtual std::uint32_t GetID() const { return id; }
	virtual const std::string& GetName() const { return name; }
	virtual const AssetType& GetType() const { return type; }

	// Setters
	virtual void SetID(std::uint32_t id) { this->id = id; }
	virtual void SetName(const std::string& name) { this->name = name; }
	virtual void SetType(const AssetType& type) { this->type = type; }

	// Static Public Functions
	// -----------------------
	static std::uint32_t GetNAssets() { return nAssets; }

private:
	// Static Private Attributes
	// -------------------------
	static std::uint32_t nAssets; // number of assets in the scene

protected:
	// Protected Attributes
	// --------------------
	std::uint32_t id; // unique identifier for the asset
	std::string name;
	AssetType type;

};