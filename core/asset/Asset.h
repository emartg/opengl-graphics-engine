/*
 * Asset.h
 * This file defines the Asset_Type enumeration and the Asset structure, which describe a file (or a folder) of
 * an assets directory that the engine recognizes as an asset: something that a user picks, organizes, and
 * places in a scene, such as a material, a model, or a texture. An asset is identified by its path inside the
 * assets directory, and the Asset_Registry finds them.
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

// Kinds of assets. Shaders are not assets of this kind (they are managed by the Shader_Library)
enum class Asset_Type
{
	MATERIAL, // a material descriptor file (<name>.material.json)
	MODEL,    // a 3D model file that Assimp can import (e.g., .obj, .gltf, .fbx)
	SHAPE,    // a basic shape descriptor file (<name>.shape.json)
	LIGHT,    // a light descriptor file (<name>.light.json); its kind (directional, point, or spot) is in the file
	SKYBOX,   // an HDR image (.hdr), a skybox descriptor file (<name>.skybox.json), or a folder with the six faces of a cubemap
	TEXTURE   // an image file (e.g., .png, .jpg)
};

// An asset of an assets directory
struct Asset
{
	std::uint32_t         id{ 0 };                     // unique among the assets of a registry, and kept while the asset exists
	std::string           name;                        // the name of the file or folder without its extension (e.g., "dynamic_glass")
	Asset_Type            type{ Asset_Type::TEXTURE }; // the kind of asset
	std::filesystem::path path;                        // the path relative to the assets directory, with "/" separators
};
