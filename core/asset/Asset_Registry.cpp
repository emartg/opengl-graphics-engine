/*
 * Asset_Registry.cpp
 * This file implements the Asset_Registry class, which finds the assets of an assets directory: it scans the
 * directory (and its subfolders) and recognizes each asset by the name of its file, without reading it.
 */

#include "Asset_Registry.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <set>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace
{
	// Returns a copy of a text in lowercase (the extensions of the files are compared without their case)
	std::string to_lowercase(std::string text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](unsigned char character) {
			return static_cast<char>(std::tolower(character));
		});
		return text;
	}

	// Returns true if an extension (in lowercase, with its dot) is one of a list
	template<std::size_t N>
	bool is_one_of(const std::string& extension, const std::array<const char*, N>& extensions)
	{
		return std::any_of(extensions.begin(), extensions.end(), [&extension](const char* candidate) { return extension == candidate; });
	}

	// Extensions of the model files that Assimp can import and that the engine lists as models
	constexpr std::array<const char*, 8> MODEL_EXTENSIONS{ ".obj", ".gltf", ".glb", ".fbx", ".dae", ".3ds", ".stl", ".ply" };

	// Extensions of the image files that the engine loads as textures (an HDR image is a skybox)
	constexpr std::array<const char*, 5> IMAGE_EXTENSIONS{ ".png", ".jpg", ".jpeg", ".bmp", ".tga" };

	// Names (without extension, in lowercase) of the six images of a cubemap
	constexpr std::array<const char*, 6> CUBEMAP_FACE_NAMES{ "posx", "negx", "posy", "negy", "posz", "negz" };

	// Returns the path in its text form with "/" separators, without a trailing separator, and with "." and ".."
	// resolved, so that paths written in different ways can be compared (an empty text is the root)
	std::string normalize(const fs::path& path)
	{
		std::string text = path.lexically_normal().generic_string();
		while (!text.empty() && text.back() == '/') text.pop_back();
		return text == "." ? std::string{} : text;
	}

	// Returns the extension of the name of a file, with its dot, taking a descriptor extension (e.g., ".material.json")
	// as a whole (empty if the name has no extension)
	std::string get_suffix(const std::string& file_name)
	{
		const std::string lowercase = to_lowercase(file_name);
		for (const char* suffix : { Asset_Registry::MATERIAL_EXTENSION,
									Asset_Registry::SHAPE_EXTENSION,
									Asset_Registry::LIGHT_EXTENSION,
									Asset_Registry::SKYBOX_EXTENSION })
			if (lowercase.ends_with(suffix))
				return file_name.substr(file_name.size() - std::string{ suffix }.size());
		return fs::path{ file_name }.extension().string();
	}

	// Returns the name of a file without its extension, taking a descriptor extension (e.g., ".material.json") as a whole
	std::string get_asset_name(const fs::path& file)
	{
		const std::string file_name = file.filename().string();
		return file_name.substr(0, file_name.size() - get_suffix(file_name).size());
	}

	// Returns true if a path is relative and stays inside the directory that it is relative to: it has no root, and
	// no ".." is left once "." and ".." are resolved
	bool is_safe_relative_path(const fs::path& path)
	{
		if (path.is_absolute() || path.has_root_name() || path.has_root_directory())
			return false;
		for (const fs::path& part : path.lexically_normal())
			if (part == "..")
				return false;
		return true;
	}

	// Returns the path of an item inside a folder, both as normalized texts (an empty folder is the directory itself)
	std::string join(const std::string& folder, const std::string& name)
	{
		return folder.empty() ? name : folder + "/" + name;
	}
} // namespace

// Public Static Methods
// ---------------------
std::optional<Asset_Type> Asset_Registry::get_type_from_file(const fs::path& file)
{
	const std::string file_name = to_lowercase(file.filename().string());

	// the descriptor extensions have two parts, so they are checked before the plain extension
	if (file_name.ends_with(MATERIAL_EXTENSION))
		return Asset_Type::MATERIAL;
	if (file_name.ends_with(SHAPE_EXTENSION))
		return Asset_Type::SHAPE;
	if (file_name.ends_with(LIGHT_EXTENSION))
		return Asset_Type::LIGHT;
	if (file_name.ends_with(SKYBOX_EXTENSION))
		return Asset_Type::SKYBOX;

	const std::string extension = to_lowercase(file.extension().string());
	if (extension == ".hdr")
		return Asset_Type::SKYBOX;
	if (is_one_of(extension, MODEL_EXTENSIONS))
		return Asset_Type::MODEL;
	if (is_one_of(extension, IMAGE_EXTENSIONS))
		return Asset_Type::TEXTURE;
	return std::nullopt;
}

bool Asset_Registry::is_skybox_folder(const fs::path& dir)
{
	std::set<std::string> faces_found;

	std::error_code error;
	for (fs::directory_iterator it{ dir, error }, end; !error && it != end; it.increment(error))
	{
		std::error_code entry_error;
		if (!it->is_regular_file(entry_error))
			continue;

		const fs::path& file = it->path();
		if (!is_one_of(to_lowercase(file.extension().string()), IMAGE_EXTENSIONS))
			continue;

		const std::string face_name = to_lowercase(file.stem().string());
		if (std::find(CUBEMAP_FACE_NAMES.begin(), CUBEMAP_FACE_NAMES.end(), face_name) != CUBEMAP_FACE_NAMES.end())
			faces_found.insert(face_name);
	}

	return faces_found.size() == CUBEMAP_FACE_NAMES.size();
}

bool Asset_Registry::is_valid_name(const std::string& name)
{
	if (name.empty() || name.size() > MAX_NAME_LENGTH)
		return false;
	if (name.front() == '.' || name.front() == ' ' || name.back() == '.' || name.back() == ' ')
		return false;

	constexpr std::string_view FORBIDDEN{ "/\\<>:\"|?*" };
	return std::none_of(name.begin(), name.end(), [&FORBIDDEN](char character) {
		return static_cast<unsigned char>(character) < 32 || FORBIDDEN.find(character) != std::string_view::npos;
	});
}

std::string Asset_Registry::to_string(Asset_Type type)
{
	switch (type)
	{
		case Asset_Type::MATERIAL: return "Material";
		case Asset_Type::MODEL: return "Model";
		case Asset_Type::SHAPE: return "Shape";
		case Asset_Type::LIGHT: return "Light";
		case Asset_Type::SKYBOX: return "Skybox";
		case Asset_Type::TEXTURE: return "Texture";
	}
	return "Unknown";
}

// Public Methods
// --------------
bool Asset_Registry::scan(const fs::path& directory)
{
	clear();

	std::error_code error;
	if (!fs::is_directory(directory, error))
		return false;
	root = directory;

	// visit the folders one after the other (without recursion in the call stack, and without following
	// symbolic links, which could lead outside the directory or form a loop)
	std::vector<fs::path> pending{ fs::path{} }; // folders to visit, relative to the root
	while (!pending.empty())
	{
		const fs::path relative_folder = pending.back();
		pending.pop_back();

		std::error_code iteration_error;
		for (fs::directory_iterator it{ root / relative_folder, iteration_error }, end; !iteration_error && it != end;
			 it.increment(iteration_error))
		{
			const std::string name = it->path().filename().string();
			if (name.empty() || name.front() == '.')
				continue; // hidden file or folder

			std::error_code entry_error;
			if (it->is_symlink(entry_error))
				continue;

			// the path relative to the root is built from its text, so that it always has "/" separators
			const fs::path relative_path{ normalize(relative_folder / name) };

			if (it->is_directory(entry_error))
			{
				if (is_skybox_folder(it->path()))
					assets.push_back({ get_or_create_id(relative_path.generic_string()), name, Asset_Type::SKYBOX, relative_path });
				else
				{
					folders.push_back(relative_path);
					pending.push_back(relative_path);
				}
			}
			else if (it->is_regular_file(entry_error))
			{
				const std::optional<Asset_Type> type = get_type_from_file(it->path());
				if (type)
					assets.push_back(
						{ get_or_create_id(relative_path.generic_string()), get_asset_name(it->path()), *type, relative_path });
			}
		}
	}

	// the order of the directory iteration depends on the file system, so the assets and folders are sorted
	std::sort(assets.begin(), assets.end(), [](const Asset& a, const Asset& b) {
		return a.path.generic_string() < b.path.generic_string();
	});
	std::sort(folders.begin(), folders.end(), [](const fs::path& a, const fs::path& b) { return a.generic_string() < b.generic_string(); });
	return true;
}

void Asset_Registry::clear()
{
	root.clear();
	assets.clear();
	folders.clear();
}

// Operations on the files of the directory
// ----------------------------------------
std::string Asset_Registry::get_unique_name(const fs::path& folder, const std::string& name) const
{
	const fs::path  directory = root / folder;
	std::error_code error;
	if (!fs::exists(directory / name, error))
		return name;

	// a folder has no extension to keep, even if its name has a dot
	const std::string suffix = fs::is_directory(directory / name, error) ? std::string{} : get_suffix(name);
	const std::string stem   = name.substr(0, name.size() - suffix.size());
	for (int number = 2;; ++number)
	{
		const std::string candidate = stem + " (" + std::to_string(number) + ")" + suffix;
		if (!fs::exists(directory / candidate, error))
			return candidate;
	}
}

bool Asset_Registry::create_folder(const fs::path& parent_folder, const std::string& name, fs::path& created, std::string& error)
{
	std::string parent;
	if (!find_folder(parent_folder, parent))
	{
		error = "The parent is not a folder of the assets";
		return false;
	}
	if (!is_valid_name(name))
	{
		error = "\"" + name + "\" is not a valid name for a folder";
		return false;
	}

	std::error_code file_error;
	if (fs::exists(root / join(parent, name), file_error))
	{
		error = "There is already an item named \"" + name + "\" in that folder";
		return false;
	}
	if (!fs::create_directory(root / join(parent, name), file_error) || file_error)
	{
		error = "The folder could not be created: " + file_error.message();
		return false;
	}

	created = fs::path{ join(parent, name) };
	return rescan(error);
}

bool Asset_Registry::rename(const fs::path& path, const std::string& new_name, fs::path& renamed, std::string& error)
{
	std::string      old_path;
	const Entry_Kind kind = find_entry(path, old_path);
	if (kind == Entry_Kind::NONE)
	{
		error = "The item is not an asset or a folder of the assets";
		return false;
	}
	if (!is_valid_name(new_name))
	{
		error = "\"" + new_name + "\" is not a valid name";
		return false;
	}

	// the extension of a file stays the same (a folder has none to keep)
	std::error_code   file_error;
	const fs::path    source   = root / old_path;
	const std::string suffix   = fs::is_regular_file(source, file_error) ? get_suffix(source.filename().string()) : std::string{};
	const std::string new_path = join(normalize(fs::path{ old_path }.parent_path()), new_name + suffix);
	if (new_path == old_path)
	{ // nothing to change
		renamed = fs::path{ old_path };
		return true;
	}

	// a name that is only written with another case is the same item on a file system that ignores the case
	const fs::path target = root / new_path;
	if (fs::exists(target, file_error) && !fs::equivalent(source, target, file_error))
	{
		error = "There is already an item named \"" + new_name + suffix + "\" in that folder";
		return false;
	}

	fs::rename(source, target, file_error);
	if (file_error)
	{
		error = "The item could not be renamed: " + file_error.message();
		return false;
	}

	retarget_ids(old_path, new_path);
	renamed = fs::path{ new_path };
	return rescan(error);
}

bool Asset_Registry::move(const fs::path& path, const fs::path& destination_folder, fs::path& moved, std::string& error)
{
	std::string      old_path;
	const Entry_Kind kind = find_entry(path, old_path);
	if (kind == Entry_Kind::NONE)
	{
		error = "The item is not an asset or a folder of the assets";
		return false;
	}
	std::string destination;
	if (!find_folder(destination_folder, destination))
	{
		error = "The destination is not a folder of the assets";
		return false;
	}
	if (kind == Entry_Kind::FOLDER && (destination == old_path || destination.starts_with(old_path + "/")))
	{
		error = "A folder cannot be moved into itself";
		return false;
	}
	if (destination == normalize(fs::path{ old_path }.parent_path()))
	{
		error = "The item is already in that folder";
		return false;
	}

	const std::string new_path = join(destination, fs::path{ old_path }.filename().string());
	std::error_code   file_error;
	if (fs::exists(root / new_path, file_error))
	{
		error = "There is already an item named \"" + fs::path{ new_path }.filename().string() + "\" in the destination";
		return false;
	}

	fs::rename(root / old_path, root / new_path, file_error);
	if (file_error)
	{
		error = "The item could not be moved: " + file_error.message();
		return false;
	}

	retarget_ids(old_path, new_path);
	moved = fs::path{ new_path };
	return rescan(error);
}

bool Asset_Registry::remove(const fs::path& path, std::string& error)
{
	std::string old_path;
	if (find_entry(path, old_path) == Entry_Kind::NONE)
	{
		error = "The item is not an asset or a folder of the assets";
		return false;
	}

	std::error_code file_error;
	fs::remove_all(root / old_path, file_error);
	if (file_error)
	{
		error = "The item could not be deleted: " + file_error.message();
		return false;
	}

	return rescan(error);
}

// Getters
// -------
const Asset* Asset_Registry::find_by_id(std::uint32_t id) const
{
	const auto found = std::find_if(assets.begin(), assets.end(), [id](const Asset& asset) { return asset.id == id; });
	return found != assets.end() ? &*found : nullptr;
}

const Asset* Asset_Registry::find_by_path(const fs::path& relative_path) const
{
	const std::string wanted = normalize(relative_path);
	const auto        found =
		std::find_if(assets.begin(), assets.end(), [&wanted](const Asset& asset) { return asset.path.generic_string() == wanted; });
	return found != assets.end() ? &*found : nullptr;
}

std::vector<const Asset*> Asset_Registry::get_assets_of_type(Asset_Type type) const
{
	std::vector<const Asset*> result;
	for (const Asset& asset : assets)
		if (asset.type == type)
			result.push_back(&asset);
	return result;
}

std::vector<const Asset*> Asset_Registry::get_assets_in_folder(const fs::path& folder, bool recursive) const
{
	const std::string wanted = normalize(folder);

	std::vector<const Asset*> result;
	for (const Asset& asset : assets)
	{
		const std::string parent = normalize(asset.path.parent_path());
		const bool        is_inside =
			parent == wanted || (recursive && (wanted.empty() || (parent.size() > wanted.size() && parent.starts_with(wanted + "/"))));
		if (is_inside)
			result.push_back(&asset);
	}
	return result;
}

// Private Methods
// ---------------
Asset_Registry::Entry_Kind Asset_Registry::find_entry(const fs::path& relative_path, std::string& normalized) const
{
	normalized.clear();
	if (root.empty() || !is_safe_relative_path(relative_path))
		return Entry_Kind::NONE;

	normalized = normalize(relative_path);
	if (normalized.empty())
		return Entry_Kind::NONE; // the directory itself

	if (find_by_path(normalized))
		return Entry_Kind::ASSET;
	const bool is_folder = std::any_of(folders.begin(), folders.end(), [&normalized](const fs::path& folder) {
		return folder.generic_string() == normalized;
	});
	return is_folder ? Entry_Kind::FOLDER : Entry_Kind::NONE;
}

bool Asset_Registry::find_folder(const fs::path& relative_path, std::string& normalized) const
{
	normalized.clear();
	if (root.empty() || !is_safe_relative_path(relative_path))
		return false;

	normalized = normalize(relative_path);
	if (normalized.empty())
		return true; // the directory itself

	return std::any_of(folders.begin(), folders.end(), [&normalized](const fs::path& folder) {
		return folder.generic_string() == normalized;
	});
}

void Asset_Registry::retarget_ids(const std::string& old_path, const std::string& new_path)
{
	// the identifiers are collected first, since the keys of the map change
	std::vector<std::pair<std::string, std::uint32_t>> moved;
	for (auto it = known_ids.begin(); it != known_ids.end();)
	{
		if (it->first == old_path || it->first.starts_with(old_path + "/"))
		{
			moved.emplace_back(new_path + it->first.substr(old_path.size()), it->second);
			it = known_ids.erase(it);
		}
		else
			++it;
	}
	for (const auto& [path, id] : moved) known_ids[path] = id;
}

bool Asset_Registry::rescan(std::string& error)
{
	// scan() clears the registry, including the root that it would read, so it is given a copy
	const fs::path directory = root;
	if (scan(directory))
		return true;

	error = "The assets directory could not be scanned again";
	return false;
}

std::uint32_t Asset_Registry::get_or_create_id(const std::string& generic_path)
{
	const auto known = known_ids.find(generic_path);
	if (known != known_ids.end())
		return known->second;

	const std::uint32_t id = next_id++;
	known_ids.emplace(generic_path, id);
	return id;
}
