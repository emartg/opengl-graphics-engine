/*
* Texture.cpp
* This file implements the Texture class (a derived class of Node),
* which is used to load a texture from a file and bind it to a unit.
* The Texture class supports both 2D textures and cubemap textures.
*/

#include "Texture.h"

#include <iostream>

#include <stb_image.h>

#include "../shader/Shader.h"

// Constructors
// ------------
Texture::Texture(const std::string& name, const std::string& path, const Texture_Type type)
	: Node(name, Node_Type::TEXTURE), // set the node type to TEXTURE
	texture_id{ load_texture_from_file(path.c_str()) }, path{ path }, texture_type{ type }
{}

Texture::Texture(const std::string& name, GLuint existing_id, Texture_Type type)
	: Node(name, Node_Type::TEXTURE), // set the node type to TEXTURE
	texture_id{ existing_id }, texture_type{ type }
{}

Texture::Texture(const std::string& name, const std::string& hdr_path, const bool as_hdr)
	: Node(name, Node_Type::TEXTURE), // set the node type to TEXTURE
	texture_id{
		as_hdr ? load_hdr_texture_from_file(hdr_path.c_str()) : load_texture_from_file(hdr_path.c_str())
	},
	path{ hdr_path }, texture_type{ as_hdr ? Texture_Type::HDR_EQUIRECTANGULAR : Texture_Type::UNDEFINED }
{
	if (as_hdr && texture_id == 0)
	{ // if the HDR texture failed to load, print an error
		std::cerr << "[ERROR::TEXTURE::Texture] Failed to load HDR texture from:\n\t"
			<< hdr_path << std::endl;
	}
}

Texture::Texture(const std::string& name, const std::vector<std::string>& faces)
	: Node(name, Node_Type::TEXTURE), // set the node type to TEXTURE
	texture_id{ load_cubemap_from_files(faces) }, texture_type{ Texture_Type::CUBEMAP }
{
	if (faces.size() == 6)
	{ // if 6 faces are provided, store their paths
		cubemap_face_paths = faces;
	}
	else
	{ // if not, print an error and set the cubemap texture id to 0
		std::cerr << "[ERROR::TEXTURE::Texture] Cubemap texture requires 6 face paths, "
			<< "but " << faces.size() << " were provided" << std::endl;
		texture_id = 0; // ensure texture id is 0 if cubemap loading failed
	}
}

// Public Methods
// --------------
GLuint Texture::load_texture_from_file(const GLchar* path)
{
	// ensure the path is valid
	if (path == nullptr)
	{ // if not, print an error and return 0
		std::cerr << "[ERROR::TEXTURE::load_texture_from_file] Provided path is null" << std::endl;
		return 0;
	}

	std::string filepath = std::string(path); // convert to std::string for easier handling

	// generate and bind the texture
	GLuint texture_id;
	glGenTextures(1, &texture_id);

	// ensure vertical flip is disabled for regular 2D textures (global stb state)
	stbi_set_flip_vertically_on_load(false);

	// load the image data using stb_image
	int width, height, component_count;
	unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &component_count, 0);
	if (data)
	{ // if the image loaded successfully, determine the format and upload it to OpenGL
		GLenum format = GL_RGB; // default format
		if (component_count == 1)
			format = GL_RED;
		else if (component_count == 3)
			format = GL_RGB;
		else if (component_count == 4)
			format = GL_RGBA;

		// bind the texture and upload the image data
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps for the texture

		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		std::cout << "[INFO::TEXTURE::load_texture_from_file] Texture loaded from:\n\t" << path << std::endl;
		stbi_image_free(data); // free image memory after uploading
	}
	else
	{ // if the image failed to load, print an error, free memory, and set texture_id to 0
		std::cerr << "[ERROR::TEXTURE::load_texture_from_file] Failed to load texture from:\n\t"
			<< path << "\n\tFailure reason: " << stbi_failure_reason() << std::endl;
		stbi_image_free(data); // free image memory
		texture_id = 0;
	}

	return texture_id;
}

GLuint Texture::load_hdr_texture_from_file(const GLchar* path)
{
	// ensure the path is valid
	if (path == nullptr)
	{ // if not, print an error and return 0
		std::cerr << "[ERROR::TEXTURE::load_hdr_texture_from_file] Provided path is null" << std::endl;
		return 0;
	}

	std::string filepath = std::string(path); // convert to std::string for easier handling

	// generate and bind the texture
	GLuint texture_id;
	glGenTextures(1, &texture_id);

	// flip only for this HDR load (flip state must not leak to subsequent standard/cubemap textures)
	stbi_set_flip_vertically_on_load(true);

	// load the HDR image data using stb_image
	int width, height, component_count;
	float* data = stbi_loadf(filepath.c_str(), &width, &height, &component_count, 0);
	if (data)
	{ // if the image loaded successfully, determine the format and upload it to OpenGL
		GLenum format = GL_RGB; // default format
		if (component_count == 1)
			format = GL_RED;
		else if (component_count == 3)
			format = GL_RGB;
		else if (component_count == 4)
			format = GL_RGBA;

		// bind the texture and upload the image data
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, format, GL_FLOAT, data);
		glGenerateMipmap(GL_TEXTURE_2D); // generate mipmaps for the texture

		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // clamp to edge for HDR
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		std::cout << "[INFO::TEXTURE::load_hdr_texture_from_file] HDR texture loaded from:\n\t"
			<< path << std::endl;
		stbi_image_free(data); // free image memory after uploading

		// restore default (no flip) so regular textures and cubemap faces are not inverted
		stbi_set_flip_vertically_on_load(false);
	}
	else
	{ // if the image failed to load, print an error, free memory, and set texture_id to 0
		std::cerr << "[ERROR::TEXTURE::load_hdr_texture_from_file] Failed to load HDR texture from:\n\t"
			<< path << "\n\tFailure reason: " << stbi_failure_reason() << std::endl;
		stbi_image_free(data); // free image memory
		texture_id = 0;

		// restore default (no flip) so regular textures and cubemap faces are not inverted
		stbi_set_flip_vertically_on_load(false);
	}

	return texture_id;
}

GLuint Texture::load_cubemap_from_files(const std::vector<std::string>& faces)
{
	// ensure exactly 6 faces are provided
	if (faces.size() != 6)
	{ // if not, print an error and return 0
		std::cerr << "[ERROR::TEXTURE::load_cubemap_from_files] Cubemap texture requires 6 face paths, "
			<< "but " << faces.size() << " were provided" << std::endl;
		return 0;
	}

	// create the cubemap texture and bind it
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

	// ensure vertical flip is disabled for cubemaps (global stb state)
	stbi_set_flip_vertically_on_load(false);

	// load each face of the cubemap using stb_image
	int width, height, component_count;
	unsigned char* data;
	for (GLuint i = 0; i < faces.size(); i++)
	{
		// load each face of the cubemap (assumes face order: right, left, top, bottom, front, back)
		data = stbi_load(faces[i].c_str(), &width, &height, &component_count, 0);
		if (data)
		{ // if the face loaded successfully, determine the format and upload it to OpenGL
			GLenum format = GL_RGB; // default format
			if (component_count == 1)
				format = GL_RED;
			else if (component_count == 3)
				format = GL_RGB;
			else if (component_count == 4)
				format = GL_RGBA;

			// upload the face to the correct cubemap face target
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format,
						 width, height, 0, format, GL_UNSIGNED_BYTE, data);

			std::cout << "[INFO::TEXTURE::load_cubemap_from_files] Loaded cubemap face " << i
				<< " from:\n\t" << faces[i] << std::endl;
			stbi_image_free(data); // free image memory after uploading
		}
		else
		{ // if any face failed to load, print an error, free memory, and return 0
			std::cerr << "[ERROR::TEXTURE::load_cubemap_from_files] Failed to load cubemap texture at:\n\t"
				<< faces[i] << "\n\tFailure reason: " << stbi_failure_reason() << std::endl;
			stbi_image_free(data); // free image memory
			texture_id = 0;
			return texture_id;
		}
	}

	// set the texture parameters for the cubemap
	// (GL_CLAMP_TO_EDGE is required to prevent conflicting seams)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// print success message and return the cubemap texture id
	std::cout << "[SUCCESS::TEXTURE::load_cubemap_from_files] Cubemap texture loaded successfully "
		"from provided faces" << std::endl;
	return texture_id;
}

void Texture::bind(GLuint unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit); // activate the specified texture unit
	// determine the target based on texture type (2D or cubemap)
	const GLenum target = (texture_type == Texture_Type::CUBEMAP) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
	glBindTexture(target, texture_id); // bind the texture to the specified unit
}

// Static Public Methods
// ---------------------
std::string Texture::texture_type_to_string(const Texture_Type type)
{
	switch (type)
	{
		case Texture_Type::DIFFUSE:
			return "DIFFUSE";
		case Texture_Type::SPECULAR:
			return "SPECULAR";
		case Texture_Type::NORMAL:
			return "NORMAL";
		case Texture_Type::HEIGHT:
			return "HEIGHT";
		case Texture_Type::AO:
			return "AO";
		case Texture_Type::EMISSIVE:
			return "EMISSIVE";
		case Texture_Type::ROUGHNESS:
			return "ROUGHNESS";
		case Texture_Type::METALNESS:
			return "METALNESS";
		case Texture_Type::AMBIENT:
			return "AMBIENT";
		case Texture_Type::OPACITY:
			return "OPACITY";
		case Texture_Type::DISPLACEMENT:
			return "DISPLACEMENT";
		case Texture_Type::LIGHTMAP:
			return "LIGHTMAP";
		case Texture_Type::REFLECTION:
			return "REFLECTION";
		case Texture_Type::CUBEMAP:
			return "CUBEMAP";
		case Texture_Type::HDR_EQUIRECTANGULAR:
			return "HDR_EQUIRECTANGULAR";
		default:
			return "UNDEFINED";
	}
}