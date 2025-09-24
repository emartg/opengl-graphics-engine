/*
* Texture.cpp
* This file implements the Texture class (a derived class of Asset),
* which is used to load a texture from a file and bind it to a unit.
* The Texture class supports both 2D textures and cubemap textures.
*/

#include <iostream>

#include <stb_image.h>

#include "Texture.h"

// Constructors
// ------------
Texture::Texture(const std::string& name, const std::string& path, const TextureType type)
	: Asset(name, AssetType::TEXTURE),
	textureId{ LoadTextureFromFile(path.c_str()) }, path{ path }, textureType{ type }
{}

Texture::Texture(const std::string& name, const std::vector<std::string>& faces)
	: Asset(name, AssetType::TEXTURE),
	textureId{ LoadCubemapFromFiles(faces) }, textureType{ TextureType::CUBEMAP }
{
	if (faces.size() == 6)
	{ // if 6 faces are provided, store their paths
		cubemapFacePaths = faces;
	}
	else
	{ // if not, print an error and set the cubemap texture ID to 0
		std::cerr << "[ERROR::TEXTURE::Texture] Cubemap texture requires 6 face paths, "
			<< "but " << faces.size() << " were provided." << std::endl;
		textureId = 0; // ensure texture ID is 0 if cubemap loading failed
	}
}

// Public Methods
// --------------
GLuint Texture::LoadTextureFromFile(const GLchar* path)
{
	// ensure the path is valid
	if (path == nullptr)
	{ // if not, print an error and return 0
		std::cerr << "[ERROR::TEXTURE::LoadTextureFromFile] Provided path is null." << std::endl;
		return 0;
	}

	std::string filepath = std::string(path); // convert to std::string for easier handling

	// generate and bind the texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// load the image data using stb_image
	int width, height, nComponents;
	unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nComponents, 0);
	if (data)
	{ // if the image loaded successfully, determine the format and upload it to OpenGL
		GLenum format;
		if (nComponents == 1)
			format = GL_RED;
		else if (nComponents == 3)
			format = GL_RGB;
		else if (nComponents == 4)
			format = GL_RGBA;

		// bind the texture and upload the image data
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps for the texture

		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		std::cout << "[SUCCESS::TEXTURE::LoadTextureFromFile] Texture loaded successfully from:\n\t"
			<< path << std::endl;
		stbi_image_free(data); // free image memory after uploading
	}
	else
	{ // if the image failed to load, print an error, free memory, and set textureID to 0
		std::cerr << "[ERROR::TEXTURE::LoadTextureFromFile] Failed to load texture from:\n\t"
			<< path << "\n\tFailure reason: " << stbi_failure_reason() << std::endl;
		stbi_image_free(data); // free image memory
		textureID = 0;
	}

	return textureID;
}

GLuint Texture::LoadCubemapFromFiles(const std::vector<std::string>& faces)
{
	// ensure exactly 6 faces are provided
	if (faces.size() != 6)
	{ // if not, print an error and return 0
		std::cerr << "[ERROR::TEXTURE::LoadCubemapFromFiles] Cubemap texture requires 6 face paths, "
			<< "but " << faces.size() << " were provided." << std::endl;
		return 0;
	}

	// create the cubemap texture and bind it
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	// load each face of the cubemap using stb_image
	int width, height, nComponents;
	unsigned char* data;
	for (GLuint i = 0; i < faces.size(); i++)
	{
		// load each face of the cubemap (assumes face order: right, left, top, bottom, front, back)
		data = stbi_load(faces[i].c_str(), &width, &height, &nComponents, 0);
		if (data)
		{ // if the face loaded successfully, determine the format and upload it to OpenGL
			GLenum format;
			if (nComponents == 1)
				format = GL_RED;
			else if (nComponents == 3)
				format = GL_RGB;
			else if (nComponents == 4)
				format = GL_RGBA;

			// upload the face to the correct cubemap face target
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format,
						 width, height, 0, format, GL_UNSIGNED_BYTE, data);

			std::cout << "[SUCCESS::TEXTURE::LoadCubemapFromFiles] Loaded cubemap face " << i
				<< " successfully from:\n\t" << faces[i] << std::endl;
			stbi_image_free(data); // free image memory after uploading
		}
		else
		{ // if any face failed to load, print an error, free memory, and return 0
			std::cerr << "[ERROR::TEXTURE::LoadCubemapFromFiles] Failed to load cubemap texture at:\n\t"
				<< faces[i] << "\n\tFailure reason: " << stbi_failure_reason() << std::endl;
			stbi_image_free(data); // free image memory
			textureID = 0;
			return textureID;
		}
	}

	// set the texture parameters for the cubemap
	// (GL_CLAMP_TO_EDGE is required to prevent conflicting seams)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// print success message and return the cubemap texture ID
	std::cout << "[SUCCESS::TEXTURE::LoadCubemapFromFiles] Cubemap texture loaded successfully "
		"from provided faces." << std::endl;
	return textureID;
}

void Texture::Bind(GLuint unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, textureId);
}