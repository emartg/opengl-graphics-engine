/*
* Texture.h
* This file defines the Texture class, which is used to load a texture from a file and bind it to a unit.
*/

#pragma once

#include <string>

#include <glad/glad.h>

enum class TextureType { DIFFUSE, SPECULAR };

class Texture
{
public:
	// Constructors
	// ------------
	Texture(const std::string& path, const TextureType type);

	// Destructor
	// ----------
	~Texture();

	// Public Methods
	// --------------
	// Getters
	GLuint GetID() const { return id; }
	const std::string& GetPath() const { return path; }
	const TextureType& GetType() const { return type; }

	// Loads a texture from a file and returns the texture ID
	GLuint LoadTextureFromFile(const GLchar* path);

	// Bind the texture to the specified unit (GL_TEXTURE0 + unit)
	void Bind(GLuint unit) const;

private:
	// Private Attributes
	// ------------------
	GLuint id;
	std::string path; // path of the texture to compare with other textures
	TextureType type;
};