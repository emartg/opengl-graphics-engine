/*
 * Asset_Selection.h
 * This file defines the Asset_Selection class, which holds the items (assets and folders of an assets directory)
 * that a user has selected in a window that lists them. An item is identified by its path relative to the assets
 * directory (with "/" separators, as the Asset_Registry gives it), so the selection can also hold folders, and it
 * can follow the items when they are moved or renamed. It implements the usual ways of selecting: only one item, one
 * more item (toggling it), and a range of items from the last one that was clicked. It needs no OpenGL context and
 * no window, so it can be used and tested without them.
 */

#pragma once

#include <string>
#include <utility>
#include <vector>

class Asset_Selection
{
public:
	// Public Methods
	// --------------
	// Selects only an item (and makes it the anchor of the ranges)
	void select(const std::string& path);

	// Adds an item to the selection, or removes it if it was selected, and makes it the anchor of the ranges
	void toggle(const std::string& path);

	// Selects the items of a list that are between the anchor and an item, both included (replacing the selection),
	// keeping the anchor. The list is the items in the order that the user sees them. If there is no anchor, or the
	// anchor is not in the list, only the item is selected (and becomes the anchor)
	void select_range(const std::vector<std::string>& ordered_paths, const std::string& path);

	// Unselects everything
	void clear();

	// Removes an item from the selection, and the items that are inside it (if it is a folder)
	void remove(const std::string& path);

	// Makes the selected items follow an item that was moved or renamed from an old path to a new one: the item
	// itself and the items that are inside it
	void retarget(const std::string& old_path, const std::string& new_path);

	// Keeps only the items for which a function returns true (e.g., the ones that still exist after a scan)
	template<typename Predicate>
	void keep_if(Predicate exists)
	{
		std::vector<std::string> kept;
		for (const std::string& path : paths)
			if (exists(path))
				kept.push_back(path);
		paths = std::move(kept);
		if (!anchor.empty() && !exists(anchor))
			anchor.clear();
	}

	// Getters
	// -------
	// Returns the selected items that are not inside another selected item (what an operation on a folder already does
	// to what it holds, e.g., moving), in the order of the selection
	std::vector<std::string> get_top_level_paths() const;

	// Returns true if an item is selected
	bool is_selected(const std::string& path) const;

	bool is_empty() const { return paths.empty(); }

	// The selected items, in the order in which they were selected (or in the order of the list of a range)
	const std::vector<std::string>& get_paths() const { return paths; }

	// The item from which a range starts (empty if there is none)
	const std::string& get_anchor() const { return anchor; }

private:
	// Private Attributes
	// ------------------
	std::vector<std::string> paths;
	std::string              anchor;
};
