/*
 * GLFW_Renderer.cpp
 * Implements an inferited class of the Renderer interface using GLFW,
 * a library for creating windows and handling input.
 */

#include "GLFW_Renderer.h"

#include <algorithm>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "core/Core.h"
#include "core/renderer/Renderer.h"
#include "core/managers/Selection_Manager.h"
#include "core/managers/Input_Manager.h"

GLFW_Renderer::GLFW_Renderer() : window{ nullptr }, gui_layer{ nullptr }, gui_initialized{ false } {}

GLFW_Renderer::~GLFW_Renderer()
{
	if (window)
	{
		std::cout << "[INFO::GLFWRENDERER::~GLFW_Renderer] Shutting down GLFW..." << std::endl;
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();
	std::cout << "[INFO::GLFWRENDERER::~GLFW_Renderer] GLFW shut down successfully" << std::endl;
}

bool GLFW_Renderer::init() const
{
	if (glfwInit() == GLFW_FALSE)
	{
		std::cerr << "[ERROR::GLFWRENDERER::init_gui] Failed to initialize GLFW" << std::endl;
		return false;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	std::cout << "[SUCCESS::GLFWRENDERER::init_gui] GLFW initialized successfully" << std::endl;
	return true;
}

bool GLFW_Renderer::create_window(int width, int height, const char* title)
{
	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!window)
	{ // if the window (or its OpenGL context) cannot be created, print an error message and return false
		// (GLFW is terminated by the destructor)
		std::cerr << "[ERROR::GLFWRENDERER::create_window] Failed to create GLFW window "
					 "(an OpenGL 4.5 core profile context is required; check that the GPU drivers are up to date)"
				  << std::endl;
		return false;
	}
	glfwMakeContextCurrent(window);

	// enable vsync to synchronize the frame rate with the monitor's refresh rate and reduce screen tearing
	glfwSwapInterval(1);
	return true;
}

void GLFW_Renderer::configure_window() const
{
	glfwSetWindowUserPointer(window, const_cast<GLFW_Renderer*>(this));
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void GLFW_Renderer::poll_io_events() const
{
	glfwPollEvents();
}

void GLFW_Renderer::swap_buffers() const
{
	glfwSwapBuffers(window);
}

bool GLFW_Renderer::should_close() const
{
	return glfwWindowShouldClose(window);
}

float GLFW_Renderer::get_time() const
{
	return glfwGetTime();
}

GLADloadproc GLFW_Renderer::get_proc_address() const
{
	// GLFW and GLAD declare the loader with different (but call-compatible) function pointer types
	return reinterpret_cast<GLADloadproc>(glfwGetProcAddress);
}

void GLFW_Renderer::set_callback_functions() const
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
}

void GLFW_Renderer::set_window_should_close() const
{
	glfwSetWindowShouldClose(window, true);
}

void GLFW_Renderer::wait_for_events() const
{
	ImGuiIO& io = ImGui::GetIO();

	// consider as interactions:
	// - mouse buttons pressed or held down (left, right, middle)
	// - any key pressed (e.g. when typing in a text field or using keyboard shortcuts)
	// - any ImGui widget active (e.g. sliders, buttons, text fields, etc.)
	bool mouse_button_down = io.MouseDown[0] || io.MouseDown[1] || io.MouseDown[2];
	bool key_pressed       = std::any_of(io.KeysData, io.KeysData + ImGuiKey_NamedKey_COUNT, [](const ImGuiKeyData& k) { return k.Down; });
	bool widget_active     = ImGui::IsAnyItemActive();
	// determine if the user is interacting based on the above conditions
	const bool interacting = mouse_button_down || key_pressed || widget_active;

	// define timeout for event waiting when interacting
	constexpr float event_wait_timeout = 1.0f / 60.0f; // ~60 Hz

	if (interacting) // while interacting, wake at ~60 Hz to maintain responsiveness
		glfwWaitEventsTimeout(event_wait_timeout);
	else // when idle, fully block until the next OS event (e.g., passive mouse motion)
		glfwWaitEvents();
}

void GLFW_Renderer::init_gui()
{
	// setup the Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// let the application configure the context (input flags, fonts, style, etc.)
	if (gui_layer)
		gui_layer->configure();

	// setup the platform (GLFW) and renderer (OpenGL 4.5 core, GLSL 450) backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450");
	gui_initialized = true;
}

void GLFW_Renderer::build_gui() const
{
	// start a new ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// draw the application-specific windows (if any)
	if (gui_layer)
		gui_layer->draw();
}

void GLFW_Renderer::render_gui() const
{
	ImGui::Render(); // generate the ImGui draw data
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GLFW_Renderer::shutdown_gui()
{
	if (gui_initialized)
	{ // check if the GUI was initialized before shutting it down
		std::cout << "[INFO::GLFWRENDERER::shutdown_gui] Shutting down GUI..." << std::endl;
		// destroy the application-specific GUI layer before the ImGui context it uses
		gui_layer.reset();
		// shut down the backends (which release their OpenGL objects) and destroy the ImGui context
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		gui_initialized = false;
	}

	std::cout << "[INFO::GLFWRENDERER::shutdown_gui] GUI shut down successfully" << std::endl;
}

void GLFW_Renderer::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// set the viewport to the new framebuffer size
	Core::get_instance()->framebuffer_size_callback(width, height);
}

void GLFW_Renderer::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	std::string button;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
		button = "MIDDLE_HOLD"; // middle mouse button hold for translation
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		button = "RMB_HOLD"; // right mouse button hold for rotation
	else
		button = ""; // no button pressed

	// delegate the cursor position callback to the Input_Manager
	Core::get_instance()->get_input_manager()->cursor_pos_callback(xpos, ypos, button);
}

void GLFW_Renderer::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// delegate the scroll callback to the Input_Manager
	Core::get_instance()->get_input_manager()->scroll_callback(xoffset, yoffset);
}

void GLFW_Renderer::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// delegate the key callback to the Input_Manager
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		Core::get_instance()->get_input_manager()->key_callback("ESC_PRESSED");
	else if (key == GLFW_KEY_DELETE && action == GLFW_PRESS)
		Core::get_instance()->get_input_manager()->key_callback("DEL_PRESSED");
	else if (action == GLFW_PRESS)
	{
		// convert the key code to a string representation
		std::string keyStr(1, static_cast<char>(key));
		Core::get_instance()->get_input_manager()->key_callback(keyStr + "_PRESSED");
	}
}

void GLFW_Renderer::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{                                 // if the left mouse button is pressed, perform object picking
		ImGuiIO& io = ImGui::GetIO(); // get the ImGui IO structure

		if (io.WantCaptureMouse)
			return; // ignore clicks over GUI

		// get the current cursor position
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		// use the actual default framebuffer size for correct Y-inversion and HiDPI support
		// - Y-flip: OpenGL's origin is at the bottom-left corner,
		//   while windowing systems have it at the top-left
		// - HiDPI: the window size in screen coordinates may differ from the framebuffer size in pixels
		//   on high-DPI displays
		int fb_width, fb_height;
		glfwGetFramebufferSize(window, &fb_width, &fb_height);

		// queue a pick request in the selection manager
		auto core = Core::get_instance();
		core->get_selection_manager()->queue_pick(xpos, ypos, fb_width, fb_height);
	}
}