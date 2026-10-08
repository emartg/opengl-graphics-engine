/*
 * Assimp_Model.h
 * This file defines the Assimp_Model class (a derived class of Model),
 * which is used to load and draw 3D models from files using the Assimp library.
 */

#pragma once

#include "Model.h"

#include <iostream>
#include <algorithm>
#include <map>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Shader;
class Mesh;
class Texture;
class Material;

// Forward declaration of enum to avoid cyclic includes
enum class Texture_Type;

class Assimp_Model : public Model
{
public:
	// Constructors
	// ------------
	// Constructor that loads a model from a file
	Assimp_Model(
		const std::string& name,
		std::string const& path,
		const glm::vec4    albedo       = DEFAULT_ALBEDO,
		const glm::vec3    position     = POSITION,
		const glm::quat    rotation     = ROTATION,
		const glm::vec3    scale        = SCALE,
		const glm::vec3    forward      = FORWARD,
		const glm::vec3    mesh_forward = FORWARD);

	// Destructor
	// ----------
	~Assimp_Model() = default;

	// Public Constants
	// ----------------
	// default albedo color of imported models: white, since it tints the textures of their materials
	static constexpr glm::vec4 DEFAULT_ALBEDO{ 1.0f };

private:
	// Private Attributes
	// ------------------
	std::string directory; // directory path of the model file for loading textures
	std::string file_name; // name of the model file (without its directory), used to name its materials

	// materials created while loading the model, by the index of their imported material (shared by its meshes)
	std::map<unsigned int, std::shared_ptr<Material>> imported_materials;

	// Private Methods
	// ---------------
	// Loads a model with supported Assimp extensions from file and stores the resulting meshes
	// in the meshes vector
	void load_assimp_model(std::string const& path);

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node
	// and repeats this process on its children nodes (if any)
	void process_node(aiNode* node, const aiScene* scene);

	// Processes a mesh and returns a shared pointer to the resulting Mesh object
	std::shared_ptr<Mesh> process_mesh(aiMesh* mesh, const aiScene* scene);

	// Loads the material textures of a mesh
	std::vector<std::shared_ptr<Texture>> load_material_textures(aiMaterial* mat, aiTextureType type, Texture_Type texture_type);
};
