/*
* Texture.h
* This file defines the Texture class (a derived class of Asset),
* which is used to load a texture from a file and bind it to a unit.
*/

#pragma once

#include <string>

#include <glad/glad.h>

enum class TextureType { DIFFUSE, SPECULAR };

#include "../Asset.h"

class Texture : public Asset
{
public:
	// Constructors
	// ------------
	Texture(const std::string& name,
			const std::string& path, const TextureType type);

	// Public Methods
	// --------------
	// Loads the texture
	void Load() override {}

	// Deallocates all the resources of the texture
	void DeallocateResources() override { glDeleteTextures(1, &id); }

	// Getters
	GLuint GetID() const { return id; }
	const std::string& GetPath() const { return path; }
	const TextureType& GetTextureType() const { return textureType; }

	// Loads a texture from a file and returns the texture ID
	GLuint LoadTextureFromFile(const GLchar* path);

	// Bind the texture to the specified unit (GL_TEXTURE0 + unit)
	void Bind(GLuint unit) const;

private:
	// Private Attributes
	// ------------------
	GLuint id;
	std::string path; // path of the texture to compare with other textures
	TextureType textureType;
};