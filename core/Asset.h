/*
* Asset.h
* This file defines the Asset class, which is an abstract base class
* to manage an asset meant to be used by the engine.
*/

#pragma once

#include <string>

enum class AssetType
{
	CAMERA,
	LIGHT,
	MODEL,
	SHADER,
	TEXTURE
};

class Asset
{
public:
	// Constructors
	// ------------
	Asset(const std::string& name, const AssetType type)
		: name{ name }, type{ type } {}

	// Virtual destructor (ensures that derived classes can be deleted properly - polymorphism)
	// ---------------------------------------------------------------------------------------
	virtual ~Asset() {}

	// Public Functions
	// ----------------
	// Loads the asset
	virtual void Load() = 0;

	// Deallocates all the resources of the asset
	virtual void DeallocateResources() = 0;

	// Getters
	virtual const std::string& GetName() const { return name; }
	virtual const AssetType& GetType() const { return type; }

	// Setters
	virtual void SetName(const std::string& name) { this->name = name; }

protected:
	// Protected Attributes (can be accessed by derived classes)
	// ---------------------------------------------------------
	std::string name;
	AssetType type;

};