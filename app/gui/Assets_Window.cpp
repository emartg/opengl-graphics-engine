/*
 * Assets_Window.cpp
 * This file implements the Assets_Window class, which draws the ASSETS window of the App: the folders and the assets
 * of the engine's assets directory, where the user selects them, creates, renames, and deletes folders and assets, and
 * moves them by dragging them onto a folder.
 */

#include "Assets_Window.h"

#include "core/Core.h"
#include "core/managers/Selection_Manager.h"
#include "core/material/Material_Library.h"

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <variant>

namespace fs = std::filesystem;

namespace
{
	// colors of the messages
	const ImVec4 ERROR_COLOR{ 1.0f, 0.4f, 0.4f, 1.0f };
	const ImVec4 SUCCESS_COLOR{ 0.6f, 0.9f, 0.6f, 1.0f };
	const ImVec4 WARNING_COLOR{ 1.0f, 0.8f, 0.4f, 1.0f };

	// type of the payload of the items of the window that are dragged
	constexpr const char* ITEMS_PAYLOAD{ "ASSET_ITEMS" };

	// maximum number of lines of a list in a dialog or in the Properties window before it is cut
	constexpr std::size_t MAX_LISTED_ITEMS{ 12 };

	// Returns the folder of a path (relative, with "/" separators), empty if it is in the directory itself
	std::string parent_of(const std::string& path)
	{
		const std::size_t separator = path.rfind('/');
		return separator == std::string::npos ? std::string{} : path.substr(0, separator);
	}

	// Returns the last part of a path
	std::string name_of(const std::string& path)
	{
		const std::size_t separator = path.rfind('/');
		return separator == std::string::npos ? path : path.substr(separator + 1);
	}

	// Returns true if a path is an item or is inside it
	bool is_same_or_inside(const std::string& path, const std::string& item)
	{
		return path == item || path.starts_with(item + "/");
	}

	// Returns a text in lowercase, to compare texts without considering the case
	std::string to_lowercase(std::string text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](unsigned char character) {
			return static_cast<char>(std::tolower(character));
		});
		return text;
	}

	// Returns a size in bytes as a text (e.g., "3.2 MB")
	std::string format_size(std::uintmax_t bytes)
	{
		constexpr const char* UNITS[]{ "B", "KB", "MB", "GB" };
		double                size{ static_cast<double>(bytes) };
		std::size_t           unit{ 0 };
		while (size >= 1024.0 && unit < 3)
		{
			size /= 1024.0;
			++unit;
		}
		std::ostringstream text;
		text << std::fixed << std::setprecision(unit == 0 ? 0 : 1) << size << " " << UNITS[unit];
		return text.str();
	}

	// Returns the value of a material parameter as a text
	std::string to_text(const Material_Value& value)
	{
		std::ostringstream text;
		std::visit(
			[&text](const auto& content) {
				using Type = std::decay_t<decltype(content)>;
				if constexpr (std::is_same_v<Type, bool>)
					text << (content ? "true" : "false");
				else if constexpr (std::is_same_v<Type, float>)
					text << content;
				else
				{ // a vector of two, three, or four floats
					for (int i = 0; i < content.length(); ++i) text << (i ? ", " : "") << content[i];
				}
			},
			value);
		return text.str();
	}

	// Returns the name of the environment mode of a material as a text
	const char* to_text(Environment_Mode mode)
	{
		switch (mode)
		{
			case Environment_Mode::SKYBOX: return "Skybox";
			case Environment_Mode::DYNAMIC: return "Dynamic";
			default: return "None";
		}
	}
} // namespace

// Public Methods
// --------------
void Assets_Window::draw(const ImVec2& position, const ImVec2& size, ImFont* bold_font)
{
	// set initial size and position for the window
	ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(position, ImGuiCond_Appearing);
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	ImGui::PushFont(bold_font);
	ImGui::Begin("ASSETS", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
	ImGui::PopFont();

	ensure_scanned();
	if (has_pending_move)
	{ // the items were dropped in the last frame: they are moved now, when no list is being drawn
		has_pending_move = false;
		move_items(dragged_paths, pending_move_folder);
	}
	draw_toolbar();
	draw_status();

	// the folders on the left and the content of the current folder on the right
	const float tree_width = ImGui::GetContentRegionAvail().x * 0.35f;
	drop_target_found      = false;
	ImGui::BeginChild("AssetFolderTree", ImVec2{ tree_width, 0.0f }, true);
	draw_folder_tree();
	ImGui::EndChild();
	refuse_on_empty_space();
	ImGui::SameLine();
	ImGui::BeginChild("AssetList", ImVec2{ 0.0f, 0.0f }, true);
	draw_list();
	ImGui::EndChild();
	refuse_on_empty_space();

	draw_dialogs();

	ImGui::End();

	if (just_appeared)
	{                                   // in the first frame the window must not take the focus
		ImGui::SetWindowFocus(nullptr); // set focus to no window
		just_appeared = false;
	}
}

void Assets_Window::draw_selection_properties(ImFont* bold_font)
{
	const std::vector<std::string>& paths = selection.get_paths();
	if (paths.empty())
		return;

	if (paths.size() > 1)
	{ // several items: the list of their names
		ImGui::PushFont(bold_font);
		ImGui::Text("%zu items selected", paths.size());
		ImGui::PopFont();
		ImGui::Separator();
		for (std::size_t i = 0; i < paths.size() && i < MAX_LISTED_ITEMS * 2; ++i) ImGui::BulletText("%s", paths[i].c_str());
		if (paths.size() > MAX_LISTED_ITEMS * 2)
			ImGui::TextDisabled("... and %zu more", paths.size() - MAX_LISTED_ITEMS * 2);
	}
	else
	{ // one item: its attributes
		const std::string& path = paths.front();
		if (path != properties_path)
			fill_properties(path);

		ImGui::PushFont(bold_font);
		ImGui::TextUnformatted(name_of(path).c_str());
		ImGui::PopFont();
		ImGui::Separator();

		for (const auto& [attribute, value] : properties_rows)
		{
			if (value.empty())
			{ // a title of a group of attributes
				ImGui::PushFont(bold_font);
				ImGui::TextUnformatted(attribute.c_str());
				ImGui::PopFont();
			}
			else
			{
				ImGui::TextDisabled("%s", attribute.c_str());
				ImGui::SameLine();
				ImGui::TextWrapped("%s", value.c_str());
			}
		}
	}

	ImGui::Separator();
	ImGui::TextDisabled("Assets are read only here.");
}

void Assets_Window::refresh()
{
	const fs::path root = registry.get_root().empty() ? Core::get_instance()->get_assets_dir() : registry.get_root();
	is_scanned          = true;
	size_cache.clear();
	properties_path.clear();
	if (!registry.scan(root))
	{
		set_status("The assets directory could not be scanned: " + root.string(), true);
		return;
	}
	selection.keep_if([this](const std::string& path) { return exists(path); });
	fix_current_folder();
}

// Private Methods
// ---------------
void Assets_Window::ensure_scanned()
{
	if (!is_scanned)
		refresh();
}

void Assets_Window::draw_toolbar()
{
	// the folder that is shown
	ImGui::TextDisabled("Folder");
	ImGui::SameLine();
	ImGui::TextUnformatted(current_folder.empty() ? "assets" : ("assets/" + current_folder).c_str());

	ImGui::BeginDisabled(current_folder.empty());
	if (ImGui::Button("Up"))
	{
		current_folder = parent_of(current_folder);
		selection.clear();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Refresh"))
		refresh();

	ImGui::SameLine();
	ImGui::TextDisabled("|");
	ImGui::SameLine();
	if (ImGui::Button("New Folder"))
		request_new_folder();
	ImGui::SameLine();
	ImGui::BeginDisabled(selection.get_paths().size() != 1);
	if (ImGui::Button("Rename"))
		request_rename();
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(selection.is_empty());
	if (ImGui::Button("Delete"))
		request_delete();
	ImGui::EndDisabled();

	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputTextWithHint("##AssetSearch", "Search assets by name", search_buffer.data(), search_buffer.size());
}

void Assets_Window::draw_status() const
{
	if (!status.empty())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, status_is_error ? ERROR_COLOR : SUCCESS_COLOR);
		ImGui::TextWrapped("%s", status.c_str());
		ImGui::PopStyleColor();
	}
	for (std::size_t i = 0; i < warnings.size() && i < MAX_LISTED_ITEMS; ++i)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, WARNING_COLOR);
		ImGui::TextWrapped("%s", warnings[i].c_str());
		ImGui::PopStyleColor();
	}
	if (warnings.size() > MAX_LISTED_ITEMS)
		ImGui::TextDisabled("... and %zu more warnings", warnings.size() - MAX_LISTED_ITEMS);
}

void Assets_Window::draw_folder_tree()
{
	draw_folder_node("", "assets");
}

void Assets_Window::draw_folder_node(const std::string& folder, const std::string& label)
{
	const std::vector<std::string> subfolders = get_subfolders(folder);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (folder == current_folder)
		flags |= ImGuiTreeNodeFlags_Selected;
	if (subfolders.empty())
		flags |= ImGuiTreeNodeFlags_Leaf;
	if (folder.empty())
		flags |= ImGuiTreeNodeFlags_DefaultOpen;

	ImGui::PushID(folder.c_str()); // the labels of folders with the same name in different places are the same
	const bool is_open = ImGui::TreeNodeEx("##folder", flags, "%s", label.c_str());
	draw_drop_target(folder);
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen() && folder != current_folder)
	{ // clicking a folder shows it
		current_folder = folder;
		selection.clear();
	}
	if (is_open)
	{
		for (const std::string& subfolder : subfolders) draw_folder_node(subfolder, name_of(subfolder));
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void Assets_Window::draw_list()
{
	const std::vector<Row> rows = get_rows();

	// the paths in the order that the user sees them, for the ranges of the selection
	std::vector<std::string> ordered_paths;
	for (const Row& row : rows) ordered_paths.push_back(row.path);

	if (ImGui::BeginTable("AssetTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
	{
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 2.0f);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 1.0f);
		ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch, 1.0f);
		// the header uses the color of the frames, because the default one is too dark for the text of the theme
		ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
		ImGui::TableHeadersRow();
		ImGui::PopStyleColor();

		for (const Row& row : rows)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::PushID(row.path.c_str());
			const bool is_selected = selection.is_selected(row.path);
			if (ImGui::Selectable(
					row.name.c_str(),
					is_selected,
					ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowOverlap))
			{
				if (row.is_folder && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{ // a double click enters a folder
					current_folder = row.path;
					selection.clear();
				}
				else
					click_row(row.path, ordered_paths);
			}

			draw_drag_source(row.path);
			if (row.is_folder)
				draw_drop_target(row.path);
			else
				draw_refusing_target("Items are dropped onto a folder, not onto an asset");

			// the menu of the right click: the item is selected first, so that the operations are about it
			if (ImGui::BeginPopupContextItem("##RowMenu"))
			{
				if (!selection.is_selected(row.path))
					click_row(row.path, ordered_paths);
				if (ImGui::MenuItem("New Folder"))
					request_new_folder();
				if (ImGui::MenuItem("Rename", nullptr, false, selection.get_paths().size() == 1))
					request_rename();
				if (ImGui::MenuItem("Delete"))
					request_delete();
				ImGui::EndPopup();
			}
			ImGui::PopID();

			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(row.type.c_str());
			ImGui::TableSetColumnIndex(2);
			if (!row.is_folder)
				ImGui::TextUnformatted(format_size(row.size).c_str());
		}
		ImGui::EndTable();
	}

	if (rows.empty())
		ImGui::TextDisabled(search_buffer[0] != '\0' ? "No asset matches the search." : "This folder is empty.");

	// clicking the empty space unselects everything
	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
		selection.clear();
}

void Assets_Window::draw_dialogs()
{
	// a dialog is opened here, in the scope of the window, from what the buttons and the menu requested
	if (requested_dialog != Dialog::NONE)
	{
		switch (requested_dialog)
		{
			case Dialog::NEW_FOLDER: ImGui::OpenPopup("New Folder###AssetsNewFolder"); break;
			case Dialog::RENAME: ImGui::OpenPopup("Rename###AssetsRename"); break;
			case Dialog::DELETE: ImGui::OpenPopup("Delete###AssetsDelete"); break;
			default: break;
		}
		requested_dialog = Dialog::NONE;
	}

	const ImVec2 center = ImGui::GetMainViewport()->GetCenter();

	// the dialogs of a name (a new folder, and the new name of an item) share their content
	const auto draw_name_dialog = [this, &center](const char* popup_id, const char* label, bool is_rename) {
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{ 0.5f, 0.5f });
		if (!ImGui::BeginPopupModal(popup_id, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		ImGui::TextUnformatted(label);
		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere();
		ImGui::SetNextItemWidth(300.0f);
		const bool confirmed_with_enter =
			ImGui::InputText("##Name", name_buffer.data(), name_buffer.size(), ImGuiInputTextFlags_EnterReturnsTrue);

		if (!dialog_error.empty())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ERROR_COLOR);
			ImGui::TextWrapped("%s", dialog_error.c_str());
			ImGui::PopStyleColor();
		}

		bool confirmed = confirmed_with_enter;
		if (ImGui::Button(is_rename ? "Rename" : "Create", ImVec2{ 120.0f, 0.0f }))
			confirmed = true;
		ImGui::SameLine();
		const bool cancelled = ImGui::Button("Cancel", ImVec2{ 120.0f, 0.0f });

		if (confirmed)
		{
			std::string error;
			const bool  done = is_rename ? rename_item(rename_path, name_buffer.data(), error) : create_folder(name_buffer.data(), error);
			dialog_error     = error;
			if (done)
				ImGui::CloseCurrentPopup();
		}
		if (cancelled)
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	};
	draw_name_dialog("New Folder###AssetsNewFolder", "Name of the new folder:", false);
	draw_name_dialog("Rename###AssetsRename", "New name (without the extension):", true);

	// the confirmation of a deletion lists what is deleted and what uses it
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{ 0.5f, 0.5f });
	if (ImGui::BeginPopupModal("Delete###AssetsDelete", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Delete %zu item(s) and everything inside them?", delete_paths.size());
		for (std::size_t i = 0; i < delete_paths.size() && i < MAX_LISTED_ITEMS; ++i) ImGui::BulletText("%s", delete_paths[i].c_str());
		if (delete_paths.size() > MAX_LISTED_ITEMS)
			ImGui::TextDisabled("... and %zu more", delete_paths.size() - MAX_LISTED_ITEMS);

		if (!delete_users.empty())
		{
			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_Text, WARNING_COLOR);
			ImGui::TextUnformatted("These assets use what is deleted, and will lose it:");
			ImGui::PopStyleColor();
			for (std::size_t i = 0; i < delete_users.size() && i < MAX_LISTED_ITEMS; ++i) ImGui::BulletText("%s", delete_users[i].c_str());
			if (delete_users.size() > MAX_LISTED_ITEMS)
				ImGui::TextDisabled("... and %zu more", delete_users.size() - MAX_LISTED_ITEMS);
		}
		ImGui::Separator();
		ImGui::TextDisabled("The files are deleted from the disk and cannot be recovered. Scenes are not checked.");

		if (ImGui::Button("Delete", ImVec2{ 120.0f, 0.0f }))
		{
			delete_items(delete_paths);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2{ 120.0f, 0.0f }))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void Assets_Window::request_new_folder()
{
	const std::string suggestion = registry.get_unique_name(current_folder, "New Folder");
	std::snprintf(name_buffer.data(), name_buffer.size(), "%s", suggestion.c_str());
	dialog_error.clear();
	requested_dialog = Dialog::NEW_FOLDER;
}

void Assets_Window::request_rename()
{
	if (selection.get_paths().size() != 1)
		return;

	rename_path = selection.get_paths().front();
	// the name of an asset is the one of its file without the extension
	const Asset* asset = registry.find_by_path(rename_path);
	std::snprintf(name_buffer.data(), name_buffer.size(), "%s", (asset ? asset->name : name_of(rename_path)).c_str());
	dialog_error.clear();
	requested_dialog = Dialog::RENAME;
}

void Assets_Window::request_delete()
{
	if (selection.is_empty())
		return;

	delete_paths = selection.get_paths();

	// the assets that use what is deleted, except the ones that are deleted too
	std::set<std::string> users;
	for (const std::string& path : delete_paths)
		for (const fs::path& user : registry.find_users(path))
		{
			const std::string user_path  = user.generic_string();
			const bool        is_deleted = std::any_of(delete_paths.begin(), delete_paths.end(), [&user_path](const std::string& deleted) {
				return is_same_or_inside(user_path, deleted);
			});
			if (!is_deleted)
				users.insert(user_path);
		}
	delete_users.assign(users.begin(), users.end());
	requested_dialog = Dialog::DELETE;
}

bool Assets_Window::create_folder(const std::string& name, std::string& error)
{
	fs::path created;
	if (!registry.create_folder(current_folder, name, created, error))
		return false;

	size_cache.clear();
	properties_path.clear();
	selection.select(created.generic_string());
	Core::get_instance()->get_selection_manager()->clear_selection(); // the selection of the App is the assets now
	set_status("Created the folder \"" + name + "\".", false);
	warnings.clear();
	return true;
}

bool Assets_Window::rename_item(const std::string& path, const std::string& name, std::string& error)
{
	fs::path renamed;
	if (!registry.rename(path, name, renamed, error))
		return false;

	const std::string new_path = renamed.generic_string();
	size_cache.clear();
	properties_path.clear();
	// what was selected and the folder that is shown follow the item
	selection.retarget(path, new_path);
	if (is_same_or_inside(current_folder, path))
		current_folder = new_path + current_folder.substr(path.size());
	warnings = registry.get_warnings();
	set_status("Renamed \"" + name_of(path) + "\" to \"" + name_of(new_path) + "\".", false);
	return true;
}

void Assets_Window::delete_items(const std::vector<std::string>& paths)
{
	std::size_t deleted = 0;
	std::string errors;
	for (const std::string& path : paths)
	{
		if (!exists(path))
			continue; // it was inside a folder that was deleted before

		std::string error;
		if (registry.remove(path, error))
		{
			++deleted;
			selection.remove(path);
		}
		else
			errors += (errors.empty() ? "" : " ") + std::string{ "\"" } + name_of(path) + "\": " + error + ".";
	}

	size_cache.clear();
	properties_path.clear();
	warnings.clear();
	fix_current_folder();
	if (errors.empty())
		set_status("Deleted " + std::to_string(deleted) + " item(s).", false);
	else
		set_status("Deleted " + std::to_string(deleted) + " item(s). " + errors, true);
}

void Assets_Window::draw_drag_source(const std::string& path)
{
	if (!ImGui::BeginDragDropSource())
		return;

	// dragging an item that is not selected drags only that item, which becomes the selection
	if (!selection.is_selected(path))
	{
		selection.select(path);
		Core::get_instance()->get_selection_manager()->clear_selection();
	}
	dragged_paths = selection.get_top_level_paths();

	const int count = static_cast<int>(dragged_paths.size());
	ImGui::SetDragDropPayload(ITEMS_PAYLOAD, &count, sizeof(count));
	if (count == 1)
		ImGui::TextUnformatted(name_of(dragged_paths.front()).c_str());
	else
		ImGui::Text("%d items", count);
	ImGui::EndDragDropSource();
}

void Assets_Window::draw_drop_target(const std::string& folder)
{
	if (!ImGui::BeginDragDropTarget())
		return;
	drop_target_found = true;

	// only the items of this window are accepted (the Scene Graph window drags nodes)
	const ImGuiPayload* dragged = ImGui::GetDragDropPayload();
	if (dragged != nullptr && dragged->IsDataType(ITEMS_PAYLOAD))
	{
		const std::string error = get_drop_error(folder);
		if (error.empty())
		{
			if (ImGui::AcceptDragDropPayload(ITEMS_PAYLOAD) != nullptr)
			{
				pending_move_folder = folder;
				has_pending_move    = true;
			}
		}
		else
			refuse_dragged_items(error);
	}
	ImGui::EndDragDropTarget();
}

void Assets_Window::draw_refusing_target(const std::string& reason)
{
	if (!ImGui::BeginDragDropTarget())
		return;
	drop_target_found = true;

	const ImGuiPayload* dragged = ImGui::GetDragDropPayload();
	if (dragged != nullptr && dragged->IsDataType(ITEMS_PAYLOAD))
		refuse_dragged_items(reason);
	ImGui::EndDragDropTarget();
}

void Assets_Window::refuse_on_empty_space()
{
	// the space of a list that is not a target: it is not a folder, so the items are refused
	if (!drop_target_found)
		draw_refusing_target("Items are dropped onto a folder of the tree or of the list");
}

void Assets_Window::refuse_dragged_items(const std::string& reason)
{
	// while the items are over the target, the reason is next to the cursor
	ImGui::SetTooltip("%s", reason.c_str());

	// the payload is only taken (without highlighting the target) to know when the items are dropped, which is when the
	// reason is also written in the status of the window, where it stays
	const ImGuiPayload* dropped =
		ImGui::AcceptDragDropPayload(ITEMS_PAYLOAD, ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
	if (dropped != nullptr && dropped->IsDelivery())
		set_status("Nothing was moved. " + reason + ".", true);
}

std::string Assets_Window::get_drop_error(const std::string& folder) const
{
	if (dragged_paths.empty())
		return "There is nothing to move";

	for (const std::string& path : dragged_paths)
	{
		const std::string error = registry.get_move_error(path, folder);
		if (!error.empty())
			return "\"" + name_of(path) + "\": " + error;
	}
	return {};
}

void Assets_Window::move_items(const std::vector<std::string>& paths, const std::string& folder)
{
	std::size_t              moved = 0;
	std::string              errors;
	std::vector<std::string> moved_warnings;
	for (const std::string& path : paths)
	{
		fs::path    new_path;
		std::string error;
		if (registry.move(path, folder, new_path, error))
		{
			++moved;
			const std::string moved_path = new_path.generic_string();
			selection.retarget(path, moved_path);
			if (is_same_or_inside(current_folder, path))
				current_folder = moved_path + current_folder.substr(path.size());
			for (const std::string& warning : registry.get_warnings()) moved_warnings.push_back(warning);
		}
		else
			errors += (errors.empty() ? "" : " ") + std::string{ "\"" } + name_of(path) + "\": " + error + ".";
	}

	size_cache.clear();
	properties_path.clear();
	warnings = moved_warnings;
	fix_current_folder();

	// the items that left the folder that is shown are no longer selected, so that an operation never acts on items that
	// the user does not see (when the search shows the items of every folder, they all stay selected)
	if (search_buffer[0] == '\0')
		selection.keep_if([this](const std::string& path) { return parent_of(path) == current_folder; });

	const std::string target = folder.empty() ? std::string{ "the assets directory" } : "\"" + name_of(folder) + "\"";
	if (errors.empty())
		set_status("Moved " + std::to_string(moved) + " item(s) to " + target + ".", false);
	else
		set_status("Moved " + std::to_string(moved) + " item(s) to " + target + ". " + errors, true);
}

void Assets_Window::click_row(const std::string& path, const std::vector<std::string>& ordered_paths)

{
	const ImGuiIO& io = ImGui::GetIO();
	if (io.KeyShift)
		selection.select_range(ordered_paths, path);
	else if (io.KeyCtrl)
		selection.toggle(path);
	else
		selection.select(path);

	// an asset that is selected is the selection of the App, so what was selected in the scene is not
	if (!selection.is_empty())
		Core::get_instance()->get_selection_manager()->clear_selection();
}

std::vector<Assets_Window::Row> Assets_Window::get_rows()
{
	std::vector<Row> rows;

	if (search_buffer[0] != '\0')
	{ // the search looks for assets in every folder
		const std::string wanted = to_lowercase(search_buffer.data());
		for (const Asset& asset : registry.get_assets())
			if (to_lowercase(asset.name).find(wanted) != std::string::npos)
				rows.push_back(
					{ asset.path.generic_string(),
					  asset.path.generic_string(),
					  Asset_Registry::to_string(asset.type),
					  false,
					  get_file_size(asset.path.generic_string()) });
		return rows;
	}

	for (const std::string& folder : get_subfolders(current_folder)) rows.push_back({ folder, name_of(folder), "Folder", true, 0 });
	for (const Asset* asset : registry.get_assets_in_folder(current_folder))
		rows.push_back(
			{ asset->path.generic_string(),
			  name_of(asset->path.generic_string()),
			  Asset_Registry::to_string(asset->type),
			  false,
			  get_file_size(asset->path.generic_string()) });
	return rows;
}

std::vector<std::string> Assets_Window::get_subfolders(const std::string& folder) const
{
	std::vector<std::string> subfolders;
	for (const fs::path& candidate : registry.get_folders())
		if (parent_of(candidate.generic_string()) == folder)
			subfolders.push_back(candidate.generic_string());
	return subfolders;
}

bool Assets_Window::is_folder(const std::string& path) const
{
	const auto& folders = registry.get_folders();
	return std::any_of(folders.begin(), folders.end(), [&path](const fs::path& folder) { return folder.generic_string() == path; });
}

bool Assets_Window::exists(const std::string& path) const
{
	return registry.find_by_path(path) != nullptr || is_folder(path);
}

std::uintmax_t Assets_Window::get_file_size(const std::string& path)
{
	const auto cached = size_cache.find(path);
	if (cached != size_cache.end())
		return cached->second;

	// a skybox that is a folder has no size of its own
	std::error_code error;
	const fs::path  file = registry.get_root() / path;
	std::uintmax_t  size = fs::is_regular_file(file, error) ? fs::file_size(file, error) : 0;
	if (error)
		size = 0;
	size_cache[path] = size;
	return size;
}

void Assets_Window::fill_properties(const std::string& path)
{
	properties_path = path;
	properties_rows.clear();

	const fs::path absolute_path = registry.get_root() / path;
	const Asset*   asset         = registry.find_by_path(path);

	properties_rows.push_back({ "Type", asset ? Asset_Registry::to_string(asset->type) : "Folder" });
	properties_rows.push_back({ "Path", path });
	properties_rows.push_back({ "Location", absolute_path.string() });
	if (asset && asset->type != Asset_Type::SKYBOX)
		properties_rows.push_back({ "Size", format_size(get_file_size(path)) });

	if (!asset)
	{ // a folder: what it holds
		properties_rows.push_back({ "Assets", std::to_string(registry.get_assets_in_folder(path, true).size()) });
		std::size_t folders = 0;
		for (const fs::path& folder : registry.get_folders())
			if (folder.generic_string().starts_with(path + "/"))
				++folders;
		properties_rows.push_back({ "Subfolders", std::to_string(folders) });
	}
	else if (asset->type == Asset_Type::SKYBOX && fs::is_directory(absolute_path))
		properties_rows.push_back({ "Cubemap", "six images (posx, negx, posy, negy, posz, negz)" });
	else if (asset->type == Asset_Type::MATERIAL)
	{ // what the descriptor of a material says
		std::ifstream      stream{ absolute_path, std::ios::binary };
		std::ostringstream text;
		text << stream.rdbuf();
		std::string error;
		const auto  descriptor = Material_Library::parse_descriptor(text.str(), absolute_path.parent_path(), error);
		if (!descriptor)
			properties_rows.push_back({ "Descriptor", "cannot be read: " + error });
		else
		{
			properties_rows.push_back({ "Material", descriptor->name });
			properties_rows.push_back({ "Shader", descriptor->shader_name });
			properties_rows.push_back({ "Environment", to_text(descriptor->environment_mode) });
			properties_rows.push_back({ "Instancing", descriptor->supports_instancing ? "supported" : "not supported" });
			if (!descriptor->parameters.empty())
			{
				properties_rows.push_back({ "Parameters", "" });
				for (const auto& [name, value] : descriptor->parameters) properties_rows.push_back({ name, to_text(value) });
			}
			if (!descriptor->texture_paths.empty())
			{
				properties_rows.push_back({ "Textures", "" });
				for (const auto& [slot, texture] : descriptor->texture_paths)
					properties_rows.push_back(
						{ slot, texture.lexically_normal().lexically_relative(registry.get_root()).generic_string() });
			}
		}
	}

	// the assets that use it
	const std::vector<fs::path> users = registry.find_users(path);
	if (!users.empty())
	{
		properties_rows.push_back({ "Used by", "" });
		for (std::size_t i = 0; i < users.size() && i < MAX_LISTED_ITEMS; ++i)
			properties_rows.push_back({ "-", users[i].generic_string() });
		if (users.size() > MAX_LISTED_ITEMS)
			properties_rows.push_back({ "-", "and " + std::to_string(users.size() - MAX_LISTED_ITEMS) + " more" });
	}
}

void Assets_Window::set_status(const std::string& text, bool is_error)
{
	status          = text;
	status_is_error = is_error;
	if (is_error)
		std::cerr << "[WARNING::ASSETS_WINDOW] " << text << std::endl;
	else
		std::cout << "[INFO::ASSETS_WINDOW] " << text << std::endl;
}

void Assets_Window::fix_current_folder()
{
	while (!current_folder.empty() && !is_folder(current_folder)) current_folder = parent_of(current_folder);
}
