/*
* GUI.h
* This file implements the GUI class, which is used to create a graphical user interface
* using the ImGui library. The engine will use this class to create a window that will
* display information about the scene and allow the user to interact with it and change
* certain parameters.
*/

#include "Gui.h"

// Constructors
// ------------
GUI::GUI()
	: m_showDemoWindow{ true }, m_showAnotherWindow{ true },
	m_clearColor{ 0.45f, 0.55f, 0.60f, 1.00f }
{}

// Destructor
// ----------
GUI::~GUI()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


void GUI::Init(GLFWwindow* window, const char* glslVersion)
{
	// setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // enable keyboard controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // enable gamepad controls

	// setup Dear ImGui style
	ImGui::StyleColorsDark();

	// setup platform/renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glslVersion);
}

void GUI::Setup()
{
	// start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// show the big demo window from Dear ImGui
	if (m_showDemoWindow)
		ImGui::ShowDemoWindow(&m_showDemoWindow);

	// show a simple window
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!"); // create a window called "Hello, world!" and append into it
		ImGui::Text("This is some useful text.");// display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &m_showDemoWindow); // edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &m_showAnotherWindow);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f); //edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&m_clearColor); // edit 3 floats representing a color

		if (ImGui::Button("Button")) // buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// show another simple window
	if (m_showAnotherWindow)
	{
		ImGui::Begin("Another Window", &m_showAnotherWindow); // pass a pointer to a bool to hold the window open/close state
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			m_showAnotherWindow = false;
		ImGui::End();
	}
}

void GUI::Render()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::Cleanup() const
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}