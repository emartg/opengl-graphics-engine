/*
* GLFW_Renderer.h
* Defines an inferited class of the Renderer interface using GLFW,
* a library for creating windows and handling input.
*/

#pragma once

#include "core/renderer/Renderer.h"

#include <iostream>

#define GLFW_INCLUDE_NONE // prevent GLFW from including OpenGL headers
#include <GLFW/glfw3.h>

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class GUI;

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
	void create_window(int width, int height, const char* title) override;
	void configure_window() const override;
	void poll_io_events() const override;
	void swap_buffers() const override;
	bool should_close() const override;

	void wait_for_events() const override;

	// Getters
	GLADloadproc get_proc_address() const override;
	float get_time() const override;

	// Setters
	void set_callback_functions() const override;
	void set_window_should_close() const override;

	// GUI
	void init_gui() override;
	void build_gui() const override;
	void render_gui() const override;
	void shutdown_gui() override;

private:
	// Private Attributes
	// ------------------
	GLFWwindow* window;
	GUI* gui;

	// Static Callback Functions
	// -------------------------
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

};