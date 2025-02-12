/*
* Model.h
* This file defines the Model class (a derived class of Asset),
* which is an abstract base class used to load a model and draw it.
*/

#pragma once

#include <vector>
#include <string>

#include <glad/glad.h>
#include <stb_image.h>

#include "../Asset.h"
#include "mesh/Mesh.h"

class Model : public Asset
{
public:
	// Constructors
	// ------------
	Model(const std::string& name)
		: Asset(name, AssetType::MODEL) {}

	// Virtual destructor (ensures that derived classes can be deleted properly - polymorphism)
	// ---------------------------------------------------------------------------------------
	virtual ~Model() {}

	// Public Functions
	// ----------------
	// Loads the model
	virtual void Load() override {}

	// Deallocates all the resources of the model
	virtual void DeallocateResources() override { for (Mesh& mesh : meshes) mesh.DeallocateResources(); }

	// Draws the model, that is, all its meshes
	virtual void Draw() const { for (const Mesh& mesh : meshes) mesh.Draw(); }

	// Draws the model, that is, all its meshes, with the specified shader (binds the textures before drawing)
	virtual void Draw(Shader& shader) const
	{
		for (const Mesh& mesh : meshes)
		{
			mesh.BindTextures(shader);
			mesh.Draw();
		}
	}

	// Binds the textures of the model
	virtual void BindTextures(Shader& shader) const { for (const Mesh& mesh : meshes) mesh.BindTextures(shader); }

protected:
	// Protected Attributes (can be accessed by derived classes)
	// ---------------------------------------------------------
	std::vector<Mesh> meshes;
	std::vector<Texture> loadedTextures;

};