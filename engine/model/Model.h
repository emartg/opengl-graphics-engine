/*
* Model.h
* This file defines the Model class, which is an abstract base class used to load a model and draw it.
*/

#pragma once

#include <vector>
#include <string>

#include <glad/glad.h>
#include <stb_image.h>

#include "../shader/Shader.h"
#include "Mesh.h"

class Model
{
public:
	// Virtual destructor (ensures that derived classes can be deleted properly - polymorphism)
	// ---------------------------------------------------------------------------------------
	virtual ~Model() { for (Mesh& mesh : meshes) mesh.DeallocateResources(); }

	// Public Functions
	// ----------------
	// Draws the model, that is, all its meshes using the provided shader (can be overridden)
	virtual void Draw(Shader& shader) const { for (const Mesh& mesh : meshes) mesh.Draw(shader); }

protected:
	// Protected Attributes (can be accessed by derived classes)
	// ---------------------------------------------------------
	std::vector<Mesh> meshes;
	std::vector<Texture> loadedTextures;

};