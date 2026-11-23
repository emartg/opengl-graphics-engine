/*
 * GLFW_Renderer.h
 * Defines an inherited class of the Renderer interface using GLFW,
 * a library for creating windows and handling input.
 * It also owns the ImGui context and its GLFW and OpenGL backends, delegating the
 * application-specific windows to a Gui_Layer (if one is set).
 */

#pragma once

#include "core/renderer/Renderer.h"

#include <iostream>
#include <memory>

#define GLFW_INCLUDE_NONE // prevent GLFW from including OpenGL headers
#include <GLFW/glfw3.h>

#include "platform/gui/Gui_Layer.h"

class GLFW_Renderer : public Renderer
{
public:
	// Constructors
	// ------------
	GLFW_Renderer();

	// Destructor
	// ----------
	~GLFW_Renderer();

	// Public Methods
	// --------------
	bool init() const override;
	bool create_window(int width, int height, const char* title) override;
	void configure_window() const override;
	void poll_io_events() const override;
	void swap_buffers() const override;
	bool should_close() const override;

	void wait_for_events() const override;

	// Getters
	GLADloadproc get_proc_address() const override;
	float        get_time() const override;

	// Setters
	void set_callback_functions() const override;
	void set_window_should_close() const override;
	// sets the application-specific GUI layer (must be called before Core::init() to be configured)
	void set_gui_layer(std::unique_ptr<Gui_Layer> gui_layer) { this->gui_layer = std::move(gui_layer); }

	// GUI
	void init_gui() override;
	void build_gui() const override;
	void render_gui() const override;
	void shutdown_gui() override;

private:
	// Private Attributes
	// ------------------
	GLFWwindow*                window;
	std::unique_ptr<Gui_Layer> gui_layer;       // application-specific GUI windows (optional)
	bool                       gui_initialized; // whether the ImGui context and backends are initialized

	// Static Callback Functions
	// -------------------------
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

	// Private Static Methods
	// ----------------------
	// Returns the position of the cursor in pixels of the framebuffer, from its top-left corner (the windowing system gives it in
	// screen coordinates, which differ from the pixels of the framebuffer on high-DPI displays)
	static void get_cursor_in_framebuffer(GLFWwindow* window, double& x, double& y);
	// Returns true if the cursor is over the scene viewport (Core::get_scene_viewport), where clicks pick, drags move the camera,
	// and the wheel dollies it
	static bool is_cursor_in_scene_viewport(GLFWwindow* window);
};
