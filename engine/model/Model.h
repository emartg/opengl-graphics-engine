/* Model.h
This file defines the Model class, which is an abstract base class used to load a model and draw it */

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
	std::string directory; // can be useful, but not strictly required
	std::vector<Texture> loadedTextures;

	// Protected Functions
	// -------------------
	// Loads a texture from a file and returns the texture ID
	GLuint LoadTextureFromFile(const char* path, const std::string& directory)
	{
		std::string filename = std::string(path);
		filename = directory + '/' + filename;

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
		}

		return textureID;
	}

};