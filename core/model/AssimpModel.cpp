/*
* AssimpModel.cpp
* This file implements the AssimpModel class (a derived class of Model),
* which is used to load an Assimp model from a file and draw it.
*/

#include "AssimpModel.h"

// Static Protected Attributes
// ---------------------------
GLuint AssimpModel::nAssimpModels{}; // initialize the number of Assimp models in the scene to 0

// Constructors
// ------------
// Constructor that loads a model from a file
AssimpModel::AssimpModel(const std::string& name,
						 const std::string& path,
						 const glm::vec3 albedo,
						 const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale,
						 const glm::vec3 forward, const glm::vec3 meshForward)
	: Model(name, albedo, position, rotation, scale, forward, meshForward, 
			ModelType::ASSIMP_MODEL) // set the model type to ASSIMP_MODEL
{
	loadAssimpModel(path);
}

// Private Methods
// ---------------
void AssimpModel::loadAssimpModel(std::string const& path)
{
	// read file via Assimp (the second argument of ReadFile is a combination of post-processing options)
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

	// check for errors in the importing process
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::Assimp::" << importer.GetErrorString() << std::endl;
		return;
	}
	// store the directory of the model file
	directory = path.substr(0, path.find_last_of('/'));

	// process the root node (recursively process all of its children)
	processNode(scene->mRootNode, scene);
}

void AssimpModel::processNode(aiNode* node, const aiScene* scene)
{
	// process each mesh located at the current node
	for (GLuint i{}; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.emplace_back(processMesh(mesh, scene));
	}
	// recursively process each of the children's nodes (if any)
	for (GLuint i{}; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene);
}

Mesh AssimpModel::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	// process vertices
	for (GLuint i{}; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		// process vertex positions, normals and texture coordinates
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex);
	}

	// process indices
	for (GLuint i{}; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (GLuint j{}; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// process material
	if (mesh->mMaterialIndex >= 0) // does the mesh contain material data?
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// load diffuse maps and add them to the textures vector
		std::vector<Texture> diffuseMaps = loadMaterialTextures(
			material, aiTextureType_DIFFUSE, TextureType::DIFFUSE
		);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		// load specular maps and add them to the textures vector
		std::vector<Texture> specularMaps = loadMaterialTextures(
			material, aiTextureType_SPECULAR, TextureType::SPECULAR
		);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	// return a mesh object created from the extracted mesh data
	return Mesh(vertices, indices, textures);
}

std::vector<Texture> AssimpModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType textureType)
{
	std::vector<Texture> textures;
	for (GLuint i{}; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		// check if the texture was loaded before and if so, continue to the next iteration
		GLboolean skip{ false };
		for (GLuint j{}; j < loadedTextures.size(); j++)
		{
			if (std::strcmp(loadedTextures[j].GetPath().data(), str.C_Str()) == 0)
			{
				// if the texture has already been loaded, add it to the textures vector
				textures.push_back(loadedTextures[j]);
				skip = true; // a texture with the same filepath has already been loaded, so no need to load it again
				break;
			}
		}
		if (!skip) // if the texture hasn't been loaded already, load it
		{
			std::string textureName{ "texture" + std::to_string(i) };
			Texture texture{ textureName, str.C_Str(), textureType };
			textures.push_back(texture);
			// to ensure we won't load the same texture again, store it in the loaded textures
			loadedTextures.push_back(texture);
		}
	}
	return textures;
}