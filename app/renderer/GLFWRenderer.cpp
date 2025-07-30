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
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();
}

bool GLFWRenderer::Init() const
{
	if (glfwInit() == GLFW_FALSE)
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return false;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	return true;
}

void GLFWRenderer::CreateWindow(int width, int height, const char* title)
{
	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
}

void GLFWRenderer::ConfigureWindow() const
{
	glfwSetWindowUserPointer(window, const_cast<GLFWRenderer*>(this));
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

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
}

float GLFWRenderer::GetTime() const { return glfwGetTime(); }

void GLFWRenderer::PollIOEvents() const { glfwPollEvents(); }

void GLFWRenderer::SwapBuffers() const { glfwSwapBuffers(window); }

bool GLFWRenderer::ShouldClose() const { return glfwWindowShouldClose(window); }

void GLFWRenderer::SetWindowShouldClose() const { glfwSetWindowShouldClose(window, true); }

void GLFWRenderer::InitGUI()
{
	gui = new GUI();
	gui->Init(window, "#version 420");
}

void GLFWRenderer::SetupGUI() const { gui->Setup(); }

void GLFWRenderer::RenderGUI() const { gui->Render(); }

void GLFWRenderer::ShutdownGUI()
{
	if (gui)
	{
		std::cout << "Shutting down GUI..." << std::endl;
		// clean up the GUI resources and shutdown the GUI
		gui->Shutdown();
		delete gui; // delete the GUI instance
		gui = nullptr; // set the GUI pointer to nullptr to avoid dangling pointer
	}
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
}