/*
 * Asset_Registry.h
 * This file defines the Asset_Registry class, which finds the assets of an assets directory: it scans the
 * directory (and its subfolders) and recognizes each asset by the name of its file, without reading it (a
 * material by its ".material.json" extension, a model by its model extension, and so on). The folders of the
 * directory are the folders of the assets, so the registry reflects what is on disk every time it scans.
 * It also creates, renames, moves, and deletes the folders and assets on disk (keeping the registry in sync).
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

	// maximum number of characters of the name of a file or a folder
	static constexpr std::size_t MAX_NAME_LENGTH{ 200 };

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

	// Returns true if a text can be the name of a file or a folder of the assets: it is not empty (nor longer than
	// MAX_NAME_LENGTH), does not start with a dot (hidden items are not assets) or with a space, does not end with a
	// dot or a space, and has no control character or any of / \ < > : " | ? * (which file systems do not accept)
	static bool is_valid_name(const std::string& name);

	// Public Methods
	// --------------
	// Scans a directory and replaces the assets and folders of the registry with the ones found in it. The
	// assets that were already known (the same path) keep their identifier, and the new ones get a new one, so
	// that identifiers are never reused. Returns false (and leaves the registry empty) if the path is not a
	// directory
	bool scan(const std::filesystem::path& root);

	// Forgets the assets and folders, but not the identifiers given so far
	void clear();

	// Operations on the files of the directory
	// ----------------------------------------
	// They change the files on disk and scan the directory again, so the pointers returned by the queries are no longer
	// valid afterwards. Every path is given relative to the scanned directory (with "/" or the native separator) and
	// must name an asset or a folder that the registry knows: an unknown file (e.g., a readme), a hidden item, a path
	// that leaves the directory (an absolute path, or one with "..") and the directory itself are refused, so nothing
	// outside the assets is ever touched. No operation overwrites an existing item: a name that is already used is an
	// error (see get_unique_name). On success, an asset or a folder that is moved or renamed keeps its identifier, and
	// so do the assets inside a folder. Every operation returns false and fills the error text on failure.
	//
	// References between assets: the materials refer to their textures with relative paths, so moving or renaming a
	// texture (or a folder with textures) rewrites the paths of the materials that use it, and moving or renaming a
	// material (or a folder with materials) rewrites its own paths, so that every material keeps its textures. A
	// material descriptor that cannot be read or written is left as it was, and a text about it is added to the
	// warnings (the operation itself succeeds). A model refers to the files next to it (e.g., its textures) in ways that
	// the registry does not read, so what is inside a folder that directly holds a model cannot be renamed or moved on
	// its own (the folder of the model can). Deleting never fails because of a reference: ask find_users first.

	// Returns a name for a new item in a folder that is not used yet: the name itself, or the name with " (2)", " (3)",
	// and so on before its extension (a descriptor extension such as ".material.json" is taken as a whole)
	std::string get_unique_name(const std::filesystem::path& folder, const std::string& name) const;

	// Creates a folder with a name in a folder (an empty path is the directory itself). Returns its path in created
	bool
	create_folder(const std::filesystem::path& parent_folder, const std::string& name, std::filesystem::path& created, std::string& error);

	// Renames an asset or a folder. The new name is the name of the asset without its extension (the extension, which
	// can be a descriptor extension such as ".material.json", stays the same). Returns the new path in renamed
	bool rename(const std::filesystem::path& path, const std::string& new_name, std::filesystem::path& renamed, std::string& error);

	// Moves an asset or a folder into another folder (an empty path is the directory itself), which cannot be the folder
	// itself or one of its subfolders. Returns the new path in moved
	bool move(
		const std::filesystem::path& path,
		const std::filesystem::path& destination_folder,
		std::filesystem::path&       moved,
		std::string&                 error);

	// Deletes an asset or a folder with everything inside it
	bool remove(const std::filesystem::path& path, std::string& error);

	// Returns the assets that use an asset or a folder (given relative to the scanned directory): the materials that
	// have a texture inside it (except the ones that are inside it too) and, for something that is inside the folder
	// of a model, the models of that folder. It is empty if nothing uses it, or if the path is not an asset or a folder
	std::vector<std::filesystem::path> find_users(const std::filesystem::path& path) const;

	// The texts about what the last operation could not update (see the references above); empty if it updated all
	const std::vector<std::string>& get_warnings() const { return warnings; }

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
	std::vector<std::string>             warnings; // what the last operation could not update

	// Private Types
	// -------------
	// What a path of the directory is for the registry
	enum class Entry_Kind
	{
		NONE,  // not an asset or a folder that the registry knows (or a path that is not valid)
		ASSET, // an asset (a file, or a folder with the six images of a cubemap)
		FOLDER // a folder of the directory
	};

	// Private Methods
	// ---------------
	// Returns the identifier of a path, giving it a new one the first time that it is seen
	std::uint32_t get_or_create_id(const std::string& generic_path);

	// Returns what a path is, and its normalized text (with "/" separators and without "." or ".."). The directory
	// itself is not an entry
	Entry_Kind find_entry(const std::filesystem::path& relative_path, std::string& normalized) const;

	// Returns true if a path is the directory itself or one of its folders (not a skybox folder), and its normalized
	// text (empty for the directory)
	bool find_folder(const std::filesystem::path& relative_path, std::string& normalized) const;

	// Returns the folder (normalized) of the nearest model that holds a path inside it, or an empty optional if the path
	// is not inside the folder of a model. The directory itself is never a folder of a model
	std::optional<std::string> find_model_folder(const std::string& normalized_path) const;

	// Rewrites the texture paths of the materials after the item at old_path was moved to new_path on disk (and before
	// the registry is scanned again), adding to the warnings the materials that could not be updated
	void update_references(const std::string& old_path, const std::string& new_path);

	// Makes the identifiers of a path and of the paths inside it follow it to a new path
	void retarget_ids(const std::string& old_path, const std::string& new_path);

	// Scans the directory again after an operation on its files, reporting a failure in the error text
	bool rescan(std::string& error);
};
