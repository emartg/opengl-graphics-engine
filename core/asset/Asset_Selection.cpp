/*
 * Asset_Selection.cpp
 * This file implements the Asset_Selection class, which holds the items (assets and folders of an assets directory)
 * that a user has selected in a window that lists them, and implements the usual ways of selecting.
 */

#include "Asset_Selection.h"

#include <algorithm>
#include <utility>

namespace
{
	// Returns true if a path is an item or is inside it (both as normalized texts)
	bool is_same_or_inside(const std::string& path, const std::string& item)
	{
		return path == item || path.starts_with(item + "/");
	}
} // namespace

// Public Methods
// --------------
void Asset_Selection::select(const std::string& path)
{
	paths  = { path };
	anchor = path;
}

void Asset_Selection::toggle(const std::string& path)
{
	const auto found = std::find(paths.begin(), paths.end(), path);
	if (found != paths.end())
		paths.erase(found);
	else
		paths.push_back(path);
	anchor = path;
}

void Asset_Selection::select_range(const std::vector<std::string>& ordered_paths, const std::string& path)
{
	const auto from = std::find(ordered_paths.begin(), ordered_paths.end(), anchor);
	const auto to   = std::find(ordered_paths.begin(), ordered_paths.end(), path);
	if (anchor.empty() || from == ordered_paths.end() || to == ordered_paths.end())
	{ // without a valid anchor (or if the item is not in the list), the item is the only one that is selected
		select(path);
		return;
	}

	// the range goes in the order of the list, whichever of the two items comes first
	const auto first = std::min(from, to);
	const auto last  = std::max(from, to);
	paths.assign(first, last + 1);
}

void Asset_Selection::clear()
{
	paths.clear();
	anchor.clear();
}

void Asset_Selection::remove(const std::string& path)
{
	std::erase_if(paths, [&path](const std::string& selected) { return is_same_or_inside(selected, path); });
	if (is_same_or_inside(anchor, path))
		anchor.clear();
}

void Asset_Selection::retarget(const std::string& old_path, const std::string& new_path)
{
	const auto follow = [&old_path, &new_path](std::string& path) {
		if (is_same_or_inside(path, old_path))
			path = new_path + path.substr(old_path.size());
	};
	for (std::string& path : paths) follow(path);
	follow(anchor);
}

// Getters
// -------
bool Asset_Selection::is_selected(const std::string& path) const
{
	return std::find(paths.begin(), paths.end(), path) != paths.end();
}
