/*
 * Assimp_Model.cpp
 * This file implements the Assimp_Model class (a derived class of Model),
 * which is used to load and draw 3D models from files using the Assimp library.
 */

#include "Assimp_Model.h"

#include "mesh/Mesh.h"
#include "../Node.h"
#include "../shader/Shader.h"
#include "../texture/Texture.h"

// Constructors
// ------------
// Constructor that loads a model from a file
Assimp_Model::Assimp_Model(
	const std::string& name,
	const std::string& path,
	const glm::vec4    albedo,
	const glm::vec3    position,
	const glm::quat    rotation,
	const glm::vec3    scale,
	const glm::vec3    forward,
	const glm::vec3    mesh_forward) :
	Model(
		name,
		Node_Type::ASSIMP_MODEL, // set the model type to ASSIMP_MODEL
		albedo,
		position,
		rotation,
		scale,
		forward,
		mesh_forward)
{
	load_assimp_model(path); // load the model from the specified path
}

// Private Methods
// ---------------
void Assimp_Model::load_assimp_model(std::string const& path)
{
	Assimp::Importer importer;

	// read file via Assimp (the second argument of ReadFile is a combination of post-processing options)
	const aiScene* scene = importer.ReadFile(
		path,
		aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
			aiProcess_PreTransformVertices // bake node transformations into vertices
	);

	// check for errors in the importing process
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "[ERROR::ASSIMPMODEL::load_assimp_model] "
				  << "Assimp failed to load model from:\n\t" << path << "\n\tImporter error: " << importer.GetErrorString() << std::endl;
		return;
	}

	// store the directory of the model file, handling both '/' and '\'
	// as separators for cross-platform compatibility
	size_t slash_fwd  = path.find_last_of('/');
	size_t slash_bwd  = path.find_last_of('\\');
	size_t last_slash = std::string::npos;

	if (slash_fwd != std::string::npos && slash_bwd != std::string::npos)
		last_slash = std::max(slash_fwd, slash_bwd);
	else
		last_slash = (slash_fwd != std::string::npos) ? slash_fwd : slash_bwd;

	if (last_slash != std::string::npos)
	{ // if a slash was found, set the directory accordingly
		// preserve the same separator as in the original path string
		char separator = path[last_slash];               // either '/' or '\'
		directory      = path.substr(0, last_slash + 1); // include the slash
	}
	else
	{
		// if no slash was found, clear the directory
		// so that textures are loaded from the current working directory
		directory.clear();
	}

	// process the root node (recursively process all of its children)
	process_node(scene->mRootNode, scene);

	std::cout << "[SUCCESS::ASSIMPMODEL::load_assimp_model] Model loaded successfully from:\n\t" << path << std::endl;
}

void Assimp_Model::process_node(aiNode* node, const aiScene* scene)
{
	// process each mesh located at the current node
	for (GLuint i{}; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.emplace_back(process_mesh(mesh, scene));
	}

	// recursively process each of the children's nodes (if any)
	for (GLuint i{}; i < node->mNumChildren; i++) process_node(node->mChildren[i], scene);
}

std::shared_ptr<Mesh> Assimp_Model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
	// vector to store vertices, indices and textures
	std::vector<Vertex> vertices; // each vertex contains position, normal and texture coordinates
	std::vector<GLuint> indices;  // each index corresponds to a vertex in the vertices vector
	// each texture corresponds to a material texture of the mesh
	std::vector<std::shared_ptr<Texture>> textures;

	// process vertices
	for (GLuint i{}; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		// process vertex positions, normals and texture coordinates (if available)
		glm::vec3 vector;
		vector.x        = mesh->mVertices[i].x;
		vector.y        = mesh->mVertices[i].y;
		vector.z        = mesh->mVertices[i].z;
		vertex.position = vector;
		vector.x        = mesh->mNormals[i].x;
		vector.y        = mesh->mNormals[i].y;
		vector.z        = mesh->mNormals[i].z;
		vertex.normal   = vector;
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			vec.x             = mesh->mTextureCoords[0][i].x;
			vec.y             = mesh->mTextureCoords[0][i].y;
			vertex.tex_coords = vec;
		}
		else // if the mesh doesn't contain texture coordinates, set them to (0, 0)
		{
			vertex.tex_coords = glm::vec2(0.0f);
		}

		// add the vertex to the vertices vector
		vertices.push_back(vertex);
	}

	// process indices
	for (GLuint i{}; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (GLuint j{}; j < face.mNumIndices; j++) indices.push_back(face.mIndices[j]);
	}

	// process material (load all supported types of texture maps)
	if (mesh->mMaterialIndex >= 0)
	{
		// retrieve the material of the mesh
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// lambda function to load and append textures of a specific type
		auto append = [&](aiTextureType ai_type, Texture_Type texture_type) {
			std::vector<std::shared_ptr<Texture>> maps = load_material_textures(material, ai_type, texture_type);
			textures.insert(textures.end(), maps.begin(), maps.end());
		};

		// load different types of textures and append them to the textures vector
		// (these are the most common types, more can be added if needed)
		append(aiTextureType_DIFFUSE, Texture_Type::DIFFUSE);
		append(aiTextureType_BASE_COLOR, Texture_Type::DIFFUSE);
		append(aiTextureType_SPECULAR, Texture_Type::SPECULAR);
		append(aiTextureType_NORMALS, Texture_Type::NORMAL);
		append(aiTextureType_HEIGHT, Texture_Type::HEIGHT);
		append(aiTextureType_AMBIENT, Texture_Type::AMBIENT);
		append(aiTextureType_EMISSIVE, Texture_Type::EMISSIVE);
		append(aiTextureType_LIGHTMAP, Texture_Type::LIGHTMAP);
		append(aiTextureType_DIFFUSE_ROUGHNESS, Texture_Type::ROUGHNESS);
		append(aiTextureType_METALNESS, Texture_Type::METALNESS);
		append(aiTextureType_DISPLACEMENT, Texture_Type::DISPLACEMENT);
		append(aiTextureType_OPACITY, Texture_Type::OPACITY);
		append(aiTextureType_REFLECTION, Texture_Type::REFLECTION);
		// unknown texture type (fallback)
		append(aiTextureType_UNKNOWN, Texture_Type::UNDEFINED);
	}

	// return a mesh object created from the extracted mesh data
	return std::make_shared<Mesh>(vertices, indices, textures);
}

std::vector<std::shared_ptr<Texture>> Assimp_Model::load_material_textures(aiMaterial* mat, aiTextureType type, Texture_Type texture_type)
{
	// vector to store the loaded textures of the specified type from the material,
	// avoiding duplicates
	std::vector<std::shared_ptr<Texture>> loaded_textures;

	GLuint texture_count = mat->GetTextureCount(type);
	std::cout << "[INFO::ASSIMPMODEL::load_material_textures] Material has " << texture_count << " texture(s) of type "
			  << Texture::texture_type_to_string(texture_type) << std::endl;

	// iterate over all textures of the specified type in the material
	// and add them to the textures vector if they haven't been loaded before
	for (GLuint i{}; i < texture_count; i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		std::cout << "[INFO::ASSIMPMODEL::load_material_textures] Raw texture path from Assimp: \"" << str.C_Str() << "\"" << std::endl;

		// check for embedded textures (those starting with '*'), which are not yet supported,
		// hence why they are not loaded and a warning is issued
		const char* path_c_string = str.C_Str();
		if (path_c_string && path_c_string[0] == '*')
		{
			// embedded texture, warn and skip loading
			std::cerr << "[WARNING::ASSIMPMODEL::load_material_textures] "
						 "Embedded textures not supported, skipping texture:\n"
					  << path_c_string << " of type " << Texture::texture_type_to_string(texture_type) << std::endl;
			// TO DO: implement support for embedded textures in the future
			// (requires extracting the texture data from the Assimp scene and creating
			// a Texture object from it without loading from file)
			continue;
		}

		std::string texture_path = directory + str.C_Str(); // construct the full path to the texture file
		std::cout << "[INFO::ASSIMPMODEL::load_material_textures] Full texture path: \"" << texture_path << "\"" << std::endl;

		// check if the texture file exists at the constructed path, and if not, try alternative paths
		std::ifstream test_file(texture_path);
		if (!test_file.good())
		{
			std::cerr << "[WARNING::ASSIMPMODEL::load_material_textures] Texture file not found: " << texture_path << std::endl;

			// texture might be just the filename without the directory,
			// so try to locate it in the model directory and common subdirectories
			std::string filename   = str.C_Str();
			size_t      last_slash = filename.find_last_of("/\\"); // strip any directory from the filename
			if (last_slash != std::string::npos)
				filename = filename.substr(last_slash + 1);

			// try multiple common subdirectories for textures within the model directory
			std::vector<std::string> search_paths = {
				directory + filename,
				directory + "Textures/" + filename,
				directory + "textures/" + filename,
				directory + "tex/" + filename,
			};

			// check each search path for the texture file and use the first one that exists
			bool found = false;
			for (const auto& search_path : search_paths)
			{
				std::ifstream test(search_path);
				if (test.good())
				{ // if the file exists at this search path, use it and break out of the loop
					texture_path = search_path;
					found        = true;
					std::cout << "[INFO::ASSIMPMODEL::load_material_textures] Found texture at: " << texture_path << std::endl;
					break;
				}
			}

			// if the texture file was not found in any of the search paths,
			// issue an error and skip loading this texture
			if (!found)
			{
				std::cerr << "[ERROR::ASSIMPMODEL::load_material_textures] Could not locate texture: " << filename << std::endl;
				continue;
			}
		}

		// check if the texture was loaded before by comparing the file paths
		// of the previously loaded textures with the current one, and if so,
		// skip loading and reuse the existing texture
		bool skip = false;
		for (const auto& cached_texture : textures)
		{
			if (cached_texture->get_texture_path() == texture_path)
			{ // if a texture with the same file path was loaded before, reuse it and break out of the loop
				loaded_textures.push_back(cached_texture);
				skip = true;
				break;
			}
		}

		// if the texture hasn't been loaded already, load it from file and add it to both
		// the textures vector (cache for the current mesh) and
		// the loaded textures vector (to be returned to the caller)
		if (!skip)
		{
			auto texture = std::make_shared<Texture>(str.C_Str(), texture_path, texture_type);
			textures.push_back(texture);
			loaded_textures.push_back(texture);
		}
	}

	return loaded_textures;
}
