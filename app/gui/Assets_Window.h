/*
 * Assets_Window.h
 * This file defines the Assets_Window class, which draws the ASSETS window of the App: the folders and the assets of
 * the engine's assets directory (the Core's assets directory), where the user selects them, creates, renames, and
 * deletes folders and assets, moves them by dragging them onto a folder, and sees the attributes of the selection
 * (read only) in the Properties window. It uses the Asset_Registry for the files and the Asset_Selection for the
 * selection, so that this class only holds what is about the window: the folder that is shown, the search text, the
 * dialogs, and the messages of the last operation.
 */

#pragma once

#include "core/asset/Asset_Registry.h"
#include "core/asset/Asset_Selection.h"

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <imgui.h>
#include <glm/glm.hpp>

class Material;
class Node;

class Assets_Window
{
public:
	// Public Methods
	// --------------
	// Draws the window at the given position and size (the place that the layout of the GUI gives it, every frame; it cannot be
	// moved, resized, or collapsed by itself), with the titles in the bold font. The window has a close button that clears
	// is_open, and nothing is drawn while it is false
	void draw(const ImVec2& position, const ImVec2& size, ImFont* bold_font, bool* is_open);

	// Draws, read only, the attributes of the selected assets and folders (for the Properties window)
	void draw_selection_properties(ImFont* bold_font);

	// Scans the assets directory again (e.g., after its files changed outside the App), keeping what is still selected
	void refresh();

	// Draws the target of the dragged assets over the scene: an invisible window over the scene viewport (its position and size
	// in the coordinates of the GUI), behind the other windows, which takes the mouse only while items of this window are dragged
	// (otherwise the clicks and the drags reach the scene). One model, shape, light, skybox, or material is accepted: the text next
	// to the cursor says what dropping it does (a model, a shape, or a light rests on the surface under the cursor, or on the ground
	// plane y = 0; a skybox replaces the skybox of the scene; a material is given to the model under the cursor), and why anything
	// else is refused. The drop only asks: place_dropped_asset does it in the next frame
	void draw_scene_drop_target(const ImVec2& position, const ImVec2& size);

	// Places the asset that was dropped onto the scene in the last frame, if any: a model is imported, and a shape or a light is
	// built from its descriptor (in the units of the scene); it rests with the bottom centre of its bounding box (of its gizmo, for a
	// light) on the point where it was dropped, and becomes the selection. A skybox replaces the skybox of the scene, and a material
	// is given to its model.
	// The result is written in the status of the window. Called at the start of a frame, before the windows are drawn
	void place_dropped_asset();

	// Takes the dragged items over a node of another window (for example an entry of the Scene Graph), inside its drop target
	// (between BeginDragDropTarget and EndDragDropTarget): one material is accepted over a model, and given to it in the next frame,
	// as when it is dropped onto the model in the scene; anything else is refused with the reason
	void accept_material_drop(const std::shared_ptr<Node>& node);

	// Getters
	// -------
	// Returns true if assets or folders are selected (an asset that is selected counts as the selection of the App)
	bool has_selection() const { return !selection.is_empty(); }

	// Setters
	// -------
	// Unselects everything (e.g., when something of the scene is selected instead)
	void clear_selection() { selection.clear(); }

private:
	// Private Types
	// -------------
	// The dialogs of the window
	enum class Dialog
	{
		NONE,
		NEW_FOLDER,
		RENAME,
		DELETE
	};

	// An asset dropped onto the scene, which place_dropped_asset places in the next frame
	struct Scene_Drop
	{
		std::string   path;                      // relative to the assets directory
		Asset_Type    type{ Asset_Type::MODEL }; // a model, a shape, a light, a skybox, or a material
		glm::vec3     point{ 0.0f };             // where a model, a shape, or a light is placed, in world units
		std::uint32_t node_id{ 0 };              // the model that a material is given to
	};

	// A row of the list of the window: a folder or an asset
	struct Row
	{
		std::string path;      // relative to the assets directory
		std::string name;      // what the row shows
		std::string type;      // "Folder" or the name of the type of the asset
		bool        is_folder; // whether the row is a folder
	};

	// Private Attributes
	// ------------------
	Asset_Registry           registry;
	Asset_Selection          selection;
	bool                     is_scanned{ false };   // whether the assets directory was scanned
	bool                     just_appeared{ true }; // whether the window is in its first frame (so it does not take the focus)
	std::string              current_folder;        // the folder whose assets are listed (relative path, empty for the directory itself)
	std::array<char, 128>    search_buffer{};       // text of the search field
	std::array<char, 128>    name_buffer{};         // text of the name field of the dialogs
	Dialog                   requested_dialog{ Dialog::NONE };        // dialog that is opened in the next frame
	std::string              dialog_error;                            // why the last attempt of the open dialog failed
	std::vector<std::string> delete_paths;                            // items that the Delete dialog asks to delete
	std::vector<std::string> delete_users;                            // assets that use the items of the Delete dialog
	std::string              rename_path;                             // item that the Rename dialog renames
	std::vector<std::string> dragged_paths;                           // items that are being dragged (the top level ones of the selection)
	std::string              pending_move_folder;                     // folder where the dropped items are moved in the next frame
	bool                     has_pending_move{ false };               // whether items were dropped and are not moved yet
	bool                     drop_target_found{ false };              // whether a target of the lists was under the cursor in this frame
	std::string              status;                                  // result of the last operation
	bool                     status_is_error{ false };                // whether the last operation failed
	std::vector<std::string> warnings;                                // what the last operation could not update (see Asset_Registry)
	std::map<std::string, std::uintmax_t>            size_cache;      // sizes of the files, so that they are not asked every frame
	std::string                                      properties_path; // item whose attributes are in properties_rows
	std::vector<std::pair<std::string, std::string>> properties_rows; // attributes of that item (name and value)

	// asset dropped onto the scene, which place_dropped_asset places in the next frame
	std::optional<Scene_Drop> pending_scene_drop;
	// what dropping the dragged items onto the scene does, or why they are refused, shown next to the cursor at the end of draw (the
	// target of the scene is drawn before the drag source, whose own text would otherwise replace it)
	std::string scene_drop_text;

	// Private Methods
	// ---------------
	// Scans the assets directory the first time that it is needed
	void ensure_scanned();

	// Draws the path of the current folder (each of its parts is a button that shows that folder, and a target of the
	// items that are dragged), the buttons, and the search field
	void draw_path_bar();
	void draw_toolbar();
	void draw_status() const;

	// Draws the list of the folders and assets of the current folder (or the assets that match the search)
	void draw_list();

	// Draws the dialogs (new folder, rename, and delete)
	void draw_dialogs();

	// Opens the dialog of an operation for the selection
	void request_new_folder();
	void request_rename();
	void request_delete();

	// Applies an operation to the files, updating the selection, the current folder, and the messages
	bool create_folder(const std::string& name, std::string& error);
	bool rename_item(const std::string& path, const std::string& name, std::string& error);
	void delete_items(const std::vector<std::string>& paths);

	// Makes the last item that was drawn a source of dragging: the dragged items are the selection (or only that item
	// if it is not selected), and they can be dropped on a folder
	void draw_drag_source(const std::string& path);

	// Makes the last item that was drawn a target of the dragged items for a folder (an empty path is the directory
	// itself): it is highlighted when the items can be moved there, and the reason is shown when they cannot (over the
	// folder while the items are dragged, and in the status of the window if they are dropped). Dropping the items asks
	// to move them (they are moved at the start of the next frame, not while the lists are drawn)
	void draw_drop_target(const std::string& folder);

	// Makes the last item that was drawn a target that refuses the dragged items, with the reason, which is shown over
	// it while the items are dragged, and in the status of the window if they are dropped (e.g., an asset, or the empty
	// space of the list)
	void draw_refusing_target(const std::string& reason);

	// Makes the empty space of a list (the last item that was drawn) a target that refuses the dragged items, if no
	// other target of the lists was under the cursor
	void refuse_on_empty_space();

	// Shows the reason why the dragged items are refused next to the cursor, and in the status when they are dropped
	void refuse_dragged_items(const std::string& reason);

	// Returns why the dragged items cannot be moved into a folder, or an empty text if all of them can
	std::string get_drop_error(const std::string& folder) const;

	// Returns why the dragged items cannot be dropped onto the scene at the cursor (at the given point of the framebuffer, in pixels
	// from its top-left corner), or an empty text if they can, in which case the drop is filled (and the action is the text that
	// tells the user what dropping does)
	std::string get_scene_drop_error(double framebuffer_x, double framebuffer_y, Scene_Drop& drop, std::string& action) const;

	// Shows the reason why the dragged items are refused over the scene next to the cursor, and in the status when they are dropped
	void refuse_scene_drop(const std::string& reason);

	// Places the shape or the light of a drop, built from its descriptor, and selects it
	void place_shape_or_light(const Scene_Drop& drop, const std::filesystem::path& file);
	// Adds to the scene a node that a drop created, selects it, and writes where it was placed in the status
	void finish_placing(const std::shared_ptr<Node>& node, const glm::vec3& point);

	// Returns why a material cannot be given to a node (none, a light's gizmo, a group, the camera...), or an empty text if it can
	static std::string get_material_target_error(const Node* node);

	// Draws a frame around the node on the screen (the rectangle that its bounding box covers in the scene viewport), to show which
	// model a dragged material is given to
	static void draw_node_highlight(const Node& node);

	// Returns the material of the library whose descriptor is the given file, loading it into the library if it is not there yet
	// (e.g., a descriptor added to the assets directory while the App runs), or nullptr (and the error) if it cannot be loaded
	std::shared_ptr<Material> find_or_load_material(const std::filesystem::path& file, std::string& error) const;

	// Moves items (and what is inside the folders) into a folder, updating the selection, the current folder, and the messages
	void move_items(const std::vector<std::string>& paths, const std::string& folder);

	// Selects a row of the list the way that the keys held by the user ask (shift: a range, control: toggling)
	void click_row(const std::string& path, const std::vector<std::string>& ordered_paths);

	// Returns the rows to show: the folders and assets of the current folder, or the assets that match the search
	std::vector<Row> get_rows();

	// Returns the subfolders of a folder (relative paths)
	std::vector<std::string> get_subfolders(const std::string& folder) const;

	// Returns true if a path is a folder of the directory, or an asset
	bool is_folder(const std::string& path) const;
	bool exists(const std::string& path) const;

	// Returns the size of a file (0 if it cannot be asked), kept in a cache until the next scan
	std::uintmax_t get_file_size(const std::string& path);

	// Fills the attributes of an item for the Properties window
	void fill_properties(const std::string& path);

	// Sets the result of an operation
	void set_status(const std::string& text, bool is_error);

	// Makes the current folder one that exists (the nearest parent if it was deleted or renamed)
	void fix_current_folder();
};
