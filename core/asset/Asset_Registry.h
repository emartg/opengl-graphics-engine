/*
 * Asset_Registry.h
 * This file defines the Asset_Registry class, which finds the assets of an assets directory: it scans the
 * directory (and its subfolders) and recognizes each asset by the name of its file, without reading it (a
 * material by its ".material.json" extension, a model by its model extension, and so on). The folders of the
 * directory are the folders of the assets, so the registry reflects what is on disk every time it scans.
 * It needs no OpenGL context, so it can be used and tested without a window.
 *
 * A skybox can be a single HDR image, a descriptor file, or a folder that holds the six images of a cubemap
 * with the names posx, negx, posy, negy, posz, and negz (any image extension): such a folder is one asset, and
 * its images are not listed as textures. Files that are not recognized (e.g., a readme or a material library
 * of a model) and hidden files and folders (whose name starts with a dot) are ignored.
 */

#pragma once

#include "Asset.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

class Asset_Registry
{
public:
	// Public Constants
	// ----------------
	// extensions of the descriptor files of the assets that are described by a file of their own
	static constexpr const char* MATERIAL_EXTENSION{ ".material.json" };
	static constexpr const char* SHAPE_EXTENSION{ ".shape.json" };
	static constexpr const char* LIGHT_EXTENSION{ ".light.json" };
	static constexpr const char* SKYBOX_EXTENSION{ ".skybox.json" };

	// Public Static Methods
	// ---------------------
	// Returns the type of asset that a file is, judging only by its name (the extension is compared without
	// considering the case), or an empty optional if the file is not an asset. It does not recognize the
	// folders of cubemap skyboxes (see is_skybox_folder)
	static std::optional<Asset_Type> get_type_from_file(const std::filesystem::path& file);

	// Returns true if a directory holds the six images of a cubemap (posx, negx, posy, negy, posz, and negz,
	// with an image extension), which makes the directory a skybox asset
	static bool is_skybox_folder(const std::filesystem::path& dir);

	// Returns the name of an asset type for the user (e.g., "Material")
	static std::string to_string(Asset_Type type);

	// Public Methods
	// --------------
	// Scans a directory and replaces the assets and folders of the registry with the ones found in it. The
	// assets that were already known (the same path) keep their identifier, and the new ones get a new one, so
	// that identifiers are never reused. Returns false (and leaves the registry empty) if the path is not a
	// directory
	bool scan(const std::filesystem::path& root);

	// Forgets the assets and folders, but not the identifiers given so far
	void clear();

	// Getters
	// -------
	// The pointers returned by the queries below point into the registry: they stay valid until the next scan or clear

	// The scanned directory (empty if the registry has not scanned one)
	const std::filesystem::path& get_root() const { return root; }

	// All the assets, sorted by path
	const std::vector<Asset>& get_assets() const { return assets; }

	// The folders of the directory (relative to it, sorted, without the root itself), including empty ones
	const std::vector<std::filesystem::path>& get_folders() const { return folders; }

	// Returns the asset with an identifier, or nullptr if there is none
	const Asset* find_by_id(std::uint32_t id) const;

	// Returns the asset at a path relative to the scanned directory, or nullptr if there is none
	const Asset* find_by_path(const std::filesystem::path& relative_path) const;

	// Returns the assets of a type, sorted by path
	std::vector<const Asset*> get_assets_of_type(Asset_Type type) const;

	// Returns the assets of a folder (given relative to the scanned directory; an empty path is the directory
	// itself), sorted by path. With recursive, the assets of its subfolders are included too
	std::vector<const Asset*> get_assets_in_folder(const std::filesystem::path& folder, bool recursive = false) const;

private:
	// Private Attributes
	// ------------------
	std::filesystem::path                root;
	std::vector<Asset>                   assets;
	std::vector<std::filesystem::path>   folders;
	std::map<std::string, std::uint32_t> known_ids; // identifier of each path (generic form) seen so far
	std::uint32_t                        next_id{ 1 };

	// Private Methods
	// ---------------
	// Returns the identifier of a path, giving it a new one the first time that it is seen
	std::uint32_t get_or_create_id(const std::string& generic_path);
};
