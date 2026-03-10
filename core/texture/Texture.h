/*
* Texture.h
* This file defines the Texture class (a derived class of Node),
* which is used to load a texture from a file and bind it to a unit.
* The Texture class supports both 2D textures and cubemap textures.
*/

#pragma once

#include "../Node.h"

#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include <glad/glad.h> // holds the OpenGL function pointers

// Forward declaration of classes to avoid cyclic includes
class Shader;

// Enumeration class for different texture types
enum class Texture_Type
{
	UNDEFINED = 0,
	DIFFUSE,
	SPECULAR,
	NORMAL,
	HEIGHT,
	AO,
	EMISSIVE,
	ROUGHNESS,
	METALNESS,
	AMBIENT,
	OPACITY,
	DISPLACEMENT,
	LIGHTMAP,
	REFLECTION,
	CUBEMAP,
	HDR_EQUIRECTANGULAR
};

class Texture : public Node
{
public:
	// Constructors
	// ------------
	// Constructor for a 2D texture from a file path (supports different TextureTypes)
	Texture(const std::string& name, const std::string& path, const Texture_Type type);

	// Internal constructor from an existing GL texture id (used after HDR to cubemap conversion)
	Texture(const std::string& name, GLuint existing_id, Texture_Type type);

	// Contructor for a cubemap texture from a single equirectangular HDR environment map,
	// that will later be converted to a cubemap (uses the internal constructor with existing_id).
	// as_hdr should be true to indicate the file is an HDR image, otherwise 
	// the file will be loaded as a standard 2D texture
	Texture(const std::string& name, const std::string& hdr_path, const bool as_hdr);

	// Constructor for a cubemap texture from 6 individual 2D face file paths
	Texture(const std::string& name, const std::vector<std::string>& faces);

	// Copy constructor and copy assignment operator (deleted - Texture is not copyable)
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	// Move constructor and move assignment operator (deleted - Node base class is not movable)
	Texture(Texture&&) = delete;
	Texture& operator=(Texture&&) = delete;

	// Public Methods
	// --------------
	// Loads the texture
	void load() override {}

	// Deallocates all the resources of the texture
	void deallocate_resources() override { glDeleteTextures(1, &texture_id); }

	// Does not draw anything by default, as a texture has no visual representation
	virtual void draw() const override {}
	// Does not draw anything by default, as a texture has no visual representation
	virtual void draw(const Shader& shader) const override {}

	// Getters
	GLuint get_texture_id() const { return texture_id; }
	const std::string& get_texture_path() const { return path; }
	const std::vector<std::string>& get_cubemap_face_paths() const { return cubemap_face_paths; }
	const Texture_Type& get_texture_type() const { return texture_type; }

	// Setters
	void set_texture_type(const Texture_Type type) { texture_type = type; }

	// Loads a texture from a file and returns the texture id
	GLuint load_texture_from_file(const GLchar* path);

	// Loads an HDR texture from a file and returns the texture id
	GLuint load_hdr_texture_from_file(const GLchar* path);

	// Loads a cubemap texture from 6 individual texture faces
	GLuint load_cubemap_from_files(const std::vector<std::string>& faces);

	// Activates the corresponding texture unit and binds the texture to it
	// (handles both 2D textures and cubemaps)
	void bind(GLuint unit) const;

	// Static Public Methods
	// ---------------------
	// Texture type to string conversion
	static std::string texture_type_to_string(const Texture_Type type);

private:
	// Private Attributes
	// ------------------
	GLuint texture_id;
	Texture_Type texture_type;
	std::string path; // path of the texture to compare with other textures
	std::vector<std::string> cubemap_face_paths; // paths of the 6 faces if this is a cubemap

	// Friend Classes
	// --------------
	friend class Renderer; // allow Renderer to access private members for HDR to cubemap conversion

};