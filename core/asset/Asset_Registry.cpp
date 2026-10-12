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
#include <system_error>

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

	// Returns the name of a file without its extension, taking a descriptor extension (e.g., ".material.json") as a whole
	std::string get_asset_name(const fs::path& file)
	{
		const std::string file_name = file.filename().string();
		const std::string lowercase = to_lowercase(file_name);
		for (const char* suffix : { Asset_Registry::MATERIAL_EXTENSION,
									Asset_Registry::SHAPE_EXTENSION,
									Asset_Registry::LIGHT_EXTENSION,
									Asset_Registry::SKYBOX_EXTENSION })
			if (lowercase.ends_with(suffix))
				return file_name.substr(0, file_name.size() - std::string{ suffix }.size());
		return file.stem().string();
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
std::uint32_t Asset_Registry::get_or_create_id(const std::string& generic_path)
{
	const auto known = known_ids.find(generic_path);
	if (known != known_ids.end())
		return known->second;

	const std::uint32_t id = next_id++;
	known_ids.emplace(generic_path, id);
	return id;
}
