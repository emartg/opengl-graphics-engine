/*
* GLFWRenderer.cpp
* Implements an inferited class of the Renderer interface using GLFW,
* a library for creating windows and handling input.
*/

#include "GLFWRenderer.h"

GLFWRenderer::GLFWRenderer() : window{ nullptr }, gui{ nullptr } {}

GLFWRenderer::~GLFWRenderer()
{
	if (window)
	{
		std::cout << "[INFO::GLFWRENDERER::~GLFWRenderer] Shutting down GLFW..." << std::endl;
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();
	std::cout << "[INFO::GLFWRENDERER::~GLFWRenderer] GLFW shut down successfully" << std::endl;
}

bool GLFWRenderer::Init() const
{
	if (glfwInit() == GLFW_FALSE)
	{
		std::cerr << "[ERROR::GLFWRENDERER::InitGUI] Failed to initialize GLFW" << std::endl;
		return false;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	std::cout << "[SUCCESS::GLFWRENDERER::InitGUI] GLFW initialized successfully" << std::endl;
	return true;
}

void GLFWRenderer::CreateWindow(int width, int height, const char* title)
{
	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!window)
	{
		std::cerr << "[ERROR::GLFWRENDERER::CreateWindow] Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
}

void GLFWRenderer::ConfigureWindow() const
{
	glfwSetWindowUserPointer(window, const_cast<GLFWRenderer*>(this));
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void GLFWRenderer::PollIOEvents() const { glfwPollEvents(); }

void GLFWRenderer::SwapBuffers() const { glfwSwapBuffers(window); }

bool GLFWRenderer::ShouldClose() const { return glfwWindowShouldClose(window); }

float GLFWRenderer::GetTime() const { return glfwGetTime(); }

const char* GLFWRenderer::GetProcAddress() const
{
	return reinterpret_cast<const char*>(glfwGetProcAddress);
}

void GLFWRenderer::SetCallbackFunctions() const
{
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
}

void GLFWRenderer::SetWindowShouldClose() const { glfwSetWindowShouldClose(window, true); }

void GLFWRenderer::WaitForEvents() const
{
	ImGuiIO& io = ImGui::GetIO();

	// consider as interactions:
	// - mouse buttons pressed or held down (left, right, middle)
	// - any key pressed (e.g. when typing in a text field or using keyboard shortcuts)
	// - any ImGui widget active (e.g. sliders, buttons, text fields, etc.)
	bool mouseButtonDown = io.MouseDown[0] || io.MouseDown[1] || io.MouseDown[2];
	bool keyPressed = std::any_of(io.KeysData, io.KeysData + ImGuiKey_NamedKey_COUNT,
								  [](const ImGuiKeyData& k) { return k.Down; });
	bool widgetActive = ImGui::IsAnyItemActive();
	// determine if the user is interacting based on the above conditions
	const bool interacting = mouseButtonDown || keyPressed || widgetActive;

	// define timeout for event waiting when interacting
	constexpr float eventWaitTimeout = 1.0f / 60.0f; // ~60 Hz

	if (interacting) // while interacting, wake at ~60 Hz to maintain responsiveness
		glfwWaitEventsTimeout(eventWaitTimeout);
	else // when idle, fully block until the next OS event (e.g., passive mouse motion)
		glfwWaitEvents();
}

void GLFWRenderer::InitGUI()
{
	gui = new GUI();
	gui->InitGUI(window, "#version 420");
}

void GLFWRenderer::BuildGUI() const { gui->BuildGUI(); }

void GLFWRenderer::RenderGUI() const { gui->RenderGUI(); }

void GLFWRenderer::ShutdownGUI()
{
	if (gui)
	{ // check if the GUI instance is not null before shutting it down
		std::cout << "[INFO::GLFWRENDERER::ShutdownGUI] Shutting down GUI..." << std::endl;
		// clean up the GUI resources and shutdown the GUI
		gui->ShutdownGUI();
		delete gui; // delete the GUI instance
		gui = nullptr; // set the GUI pointer to nullptr to avoid dangling pointer
	}

	std::cout << "[INFO::GLFWRENDERER::ShutdownGUI] GUI shut down successfully" << std::endl;
}

void GLFWRenderer::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	// set the viewport to the new framebuffer size
	Core::GetInstance()->FramebufferSizeCallback(width, height);
}

void GLFWRenderer::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	std::string button;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		button = "RMB_HOLD"; // right mouse button hold for rotation
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		button = "LMB_HOLD"; // left mouse button hold for translation
	else
		button = ""; // no button pressed

	// delegate the cursor position callback to the InputManager
	Core::GetInstance()->GetInputManager()->CursorPosCallback(xpos, ypos, button);
}

void GLFWRenderer::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	// delegate the scroll callback to the InputManager
	Core::GetInstance()->GetInputManager()->ScrollCallback(xoffset, yoffset);
}

void GLFWRenderer::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// delegate the key callback to the InputManager
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		Core::GetInstance()->GetInputManager()->KeyCallback("ESC_PRESSED");
	else if (key == GLFW_KEY_R && action == GLFW_PRESS)
		Core::GetInstance()->GetInputManager()->KeyCallback("R_PRESSED");
	else if (key == GLFW_KEY_T && action == GLFW_PRESS)
		Core::GetInstance()->GetInputManager()->KeyCallback("T_PRESSED");
	else if (key == GLFW_KEY_DELETE && action == GLFW_PRESS)
		Core::GetInstance()->GetInputManager()->KeyCallback("DEL_PRESSED");
}

void GLFWRenderer::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{ // if the left mouse button is pressed, perform object picking
		ImGuiIO& io = ImGui::GetIO(); // get the ImGui IO structure

		if (io.WantCaptureMouse)
			return; // ignore clicks over GUI

		// get the current cursor position
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		// queue a pick request in the SelectionManager
		auto core = Core::GetInstance();
		core->GetSelectionManager()->QueuePick(
			xpos, ypos,
			core->GetScreenWidth(), core->GetScreenHeight()
		);
	}
}