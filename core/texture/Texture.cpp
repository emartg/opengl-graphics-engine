/*
* Texture.cpp
* This file implements the Texture class, which is used to load a texture from a file and bind it to a unit.
*/

#include <iostream>

#include <stb_image.h>

#include "Texture.h"

// Constructors
// ------------
Texture::Texture(const std::string& path, const TextureType type)
	: id{ 0 }, type{ type }, path{ path }
{
	id = LoadTextureFromFile(path.c_str());
}

// Destructor
// ----------
Texture::~Texture()
{
	glDeleteTextures(1, &id);
}

// Public Methods
// --------------
GLuint Texture::LoadTextureFromFile(const GLchar* path)
{
	std::string filename = std::string(path);

	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height, nComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nComponents, 0);
	if (data)
	{
		GLenum format;
		if (nComponents == 1)
			format = GL_RED;
		else if (nComponents == 3)
			format = GL_RGB;
		else if (nComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cerr << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
		textureID = 0; // return 0 if texture failed to load
	}

	return textureID;
}

void Texture::Bind(GLuint unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, id);
}