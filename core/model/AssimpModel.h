/*
* AssimpModel.h
* This file defines the AssimpModel class (a derived class of Model),
* which is used to load and draw 3D models from files using the Assimp library.
*/

#pragma once

#include "Model.h"

#include <iostream>
#include <algorithm>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Shader;
class Mesh;
class Texture;

// Forward declaration of enum to avoid cyclic includes
enum class TextureType;

class AssimpModel : public Model
{
public:
	// Constructors
	// ------------
	// Constructor that loads a model from a file
	AssimpModel(const std::string& name, std::string const& path,
				const glm::vec4 albedo = ALBEDO, const glm::vec3 position = POSITION,
				const glm::quat rotation = ROTATION, const glm::vec3 scale = SCALE,
				const glm::vec3 forward = FORWARD, const glm::vec3 meshForward = FORWARD);

	// Destructor
	// ----------
	~AssimpModel() { nAssimpModels--; } // decrements the number of Assimp models

	// Static Public Methods
	// ---------------------
	static GLuint GetNAssimpModels() { return nAssimpModels; }

private:
	// Static Private Attributes
	// -------------------------
	static GLuint nAssimpModels; // number of Assimp models in the scene

	// Private Methods
	// ---------------
	// Loads a model with supported Assimp extensions from file and stores the resulting meshes 
	// in the meshes vector
	void loadAssimpModel(std::string const& path);

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node 
	// and repeats this process on its children nodes (if any)
	void processNode(aiNode* node, const aiScene* scene);

	// Processes a mesh and returns a shared pointer to the resulting Mesh object
	std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene);

	// Loads the material textures of a mesh
	std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
															   TextureType textureType);

	// Calculates the bounding box of the model based on the vertices of the meshes
	void calculateBoundingBox();

};