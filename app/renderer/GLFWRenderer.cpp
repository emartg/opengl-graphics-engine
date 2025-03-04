/*
* GLFWRenderer.cpp
* Implements an inferited class of the Renderer interface using GLFW,
* a library for creating windows and handling input.
*/

#include "GLFWRenderer.h"

GLFWRenderer::GLFWRenderer() : window{ nullptr }, gui{ nullptr } {}

GLFWRenderer::~GLFWRenderer()
{
	if (gui)
		delete gui;

	if (window)
		glfwDestroyWindow(window);
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
}

void GLFWRenderer::SetViewport(int width, int height) { glViewport(0, 0, width, height); }

float GLFWRenderer::GetTime() { return glfwGetTime(); }

void GLFWRenderer::PollIOEvents() { glfwPollEvents(); }

void GLFWRenderer::SwapBuffers() { glfwSwapBuffers(window); }

void GLFWRenderer::SetClearColor(float r, float g, float b, float a) { glClearColor(r, g, b, a); }

void GLFWRenderer::ClearBuffers() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

bool GLFWRenderer::ShouldClose() { return glfwWindowShouldClose(window); }

void GLFWRenderer::SetWindowShouldClose() { glfwSetWindowShouldClose(window, true); }

const std::string GLFWRenderer::ProcessKeyboardInput()
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		return "Esc_pressed";
	else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		return "R_pressed";
	return "";
}

void GLFWRenderer::InitGUI()
{
	gui = new GUI();
	gui->Init(window, "#version 420");
}

void GLFWRenderer::SetupGUI() { gui->Setup(); }

void GLFWRenderer::RenderGUI() { gui->Render(); }

void GLFWRenderer::CleanupGUI() { gui->Cleanup(); }

void GLFWRenderer::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	Core::GetInstance()->FramebufferSizeCallback(width, height);
}

void GLFWRenderer::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	std::string button;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		button = "right_mouse_button_pressed";
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		button = "left_mouse_button_pressed";
	else
		button = "no_mouse_button_pressed";
	Core::GetInstance()->CursorPosCallback(xpos, ypos, button);
}

void GLFWRenderer::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Core::GetInstance()->ScrollCallback(xoffset, yoffset);
}