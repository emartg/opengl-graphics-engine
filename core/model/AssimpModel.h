/*
* AssimpModel.h
* This file defines the AssimpModel class (a derived class of Model),
* which is used to load an Assimp model from a file and draw it.
*/

#pragma once

#include <algorithm>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../texture/Texture.h"
#include "../shader/Shader.h"
#include "Model.h"

class AssimpModel : public Model
{
public:
	// Constructors
	// ------------
	// Constructor that loads a model from a file
	AssimpModel(const std::string& name, std::string const& path,
				const glm::vec3 albedo = ALBEDO,
				const glm::vec3 position = POSITION, const glm::quat rotation = ROTATION,
				const glm::vec3 scale = SCALE,
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

	// Processes a mesh and returns a mesh object
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	// Loads the material textures of a mesh
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat,
											  aiTextureType type,
											  TextureType textureType);

	// Calculates the bounding box of the model based on the vertices of the meshes
	void calculateBoundingBox();

};