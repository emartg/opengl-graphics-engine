/*
* Texture.h
* This file defines the Texture class (a derived class of Asset),
* which is used to load a texture from a file and bind it to a unit.
* The Texture class supports both 2D textures and cubemap textures.
*/

#pragma once

#include <vector>
#include <string>

#include <glad/glad.h> // holds the OpenGL function pointers

#include "../Asset.h"

enum class TextureType { UNDEFINED = 0, DIFFUSE, SPECULAR, CUBEMAP };

class Texture : public Asset
{
public:
	// Constructors
	// ------------
	// Constructor for a 2D texture from a file path (supports different TextureTypes)
	Texture(const std::string& name, const std::string& path, const TextureType type);

	// Constructor for a cubemap texture from 6 individual 2D face file paths
	Texture(const std::string& name, const std::vector<std::string>& faces);

	// Public Methods
	// --------------
	// Loads the texture
	void Load() override {}

	// Deallocates all the resources of the texture
	void DeallocateResources() override { glDeleteTextures(1, &textureId); }

	// Getters
	GLuint GetTextureId() const { return textureId; }
	const std::string& GetPath() const { return path; }
	const std::vector<std::string>& GetCubemapFacePaths() const { return cubemapFacePaths; }
	const TextureType& GetTextureType() const { return textureType; }

	// Setters
	void SetTextureType(const TextureType type) { textureType = type; }

	// Loads a texture from a file and returns the texture ID
	GLuint LoadTextureFromFile(const GLchar* path);

	// Loads a cubemap texture from 6 individual texture faces
	GLuint LoadCubemapFromFiles(const std::vector<std::string>& faces);

	// Bind the texture to the specified unit (GL_TEXTURE0 + unit)
	void Bind(GLuint unit) const;

private:
	// Private Attributes
	// ------------------
	GLuint textureId;
	TextureType textureType;
	std::string path; // path of the texture to compare with other textures
	std::vector<std::string> cubemapFacePaths; // paths of the 6 faces if this is a cubemap

};