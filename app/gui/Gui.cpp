/*
* GUI.h
* This file implements the GUI class, which is used to create a graphical user interface
* using the ImGui library.
* The engine will use this class to create a window that will display information about the scene
* and allow the user to interact with it and change certain parameters.
*/

#include "Gui.h"
#include "../CUBE.h"

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

	// set initial window position to the top-left corner (with some padding)
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	// set initial window size to 320x200 pixels
	ImGui::SetNextWindowSize(ImVec2(320, 200), ImGuiCond_FirstUseEver);

	// show a window that displays all the assets in the scene and allows the user to create a new cube
	{
		ImGui::Begin("Scene Assets");

		// get the camera object
		auto& camera = Core::GetInstance()->GetCamera();
		// display the name of the camera object and its position
		ImGui::Text("Camera Name: %s", camera->GetName().c_str());
		ImGui::Text("Camera Position: (%.3f, %.3f, %.3f)",
					camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z);

		// get the position of the light source
		auto lightPos = Core::GetInstance()->GetLightPos();
		// display the position of the light source
		ImGui::Text("Light Position: (%.3f, %.3f, %.3f)", lightPos.x, lightPos.y, lightPos.z);

		// get the number of cubes in the scene via GetNCubes 
		// (provisional, as this number of cubes comes from the size of the cubePositions vector)
		unsigned int nCubes = Core::GetInstance()->GetNCubes();
		// display the number of cubes in the scene
		ImGui::Text("Number of Cubes: %d", nCubes);
		// drop-down list of the cube positions and colors
		for (GLuint i{}; i < nCubes; i++)
		{
			// display the position of the i-th cube
			ImGui::Text("Cube %d Position: (%.3f, %.3f, %.3f)", i,
						Core::GetInstance()->GetCubePos(i).x,
						Core::GetInstance()->GetCubePos(i).y,
						Core::GetInstance()->GetCubePos(i).z);
			// display the color of the i-th cube
			ImGui::Text("Cube %d Color: (%.3f, %.3f, %.3f)", i,
						Core::GetInstance()->GetCubeColor(i).x,
						Core::GetInstance()->GetCubeColor(i).y,
						Core::GetInstance()->GetCubeColor(i).z);
		}

		// button to add a new cube to the scene
		if (ImGui::Button("Add Cube"))
		{
			// create a new cube at a random position within a certain range 
			// and ensure it is not too close to the camera
			glm::vec3 newPos;
			do
			{
				newPos.x = static_cast<float>(rand() % 10 - 5);
				newPos.y = static_cast<float>(rand() % 10 - 5);
				newPos.z = static_cast<float>(rand() % 10 - 5);
			} while (glm::length(newPos - camera->GetPosition()) < 2.0f);
			// add the new position to the vector of cube positions
			Core::GetInstance()->AddCubePos(newPos);
			// add a new random color to the vector of cube colors
			Core::GetInstance()->AddCubeColor(glm::vec3(static_cast<float>(rand()) / RAND_MAX,
														static_cast<float>(rand()) / RAND_MAX,
														static_cast<float>(rand()) / RAND_MAX));

			// create a new cube with default vertices and indices and add it to the scene
			auto newCube = std::make_unique<Shape>("Cube " + std::to_string(nCubes), verticesVec, indicesVec);
			Core::GetInstance()->AddAsset(std::move(newCube));
		}

		ImGui::End();
	}

    // set initial window position to the top-right corner (with some padding)
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 340, 10), ImGuiCond_FirstUseEver);
	// set initial window size to 330x200 pixels
	ImGui::SetNextWindowSize(ImVec2(330, 200), ImGuiCond_FirstUseEver);

	// show a window that allows the user to change the color of the cubes in the scene and
	// the position of the light source
	{
		ImGui::Begin("Scene Settings");

		// get reference to the position of the light source
		auto& lightPos = Core::GetInstance()->GetLightPos();
		// create a slider for the x, y, and z components of the light position
		ImGui::SliderFloat3("Light Position", (float*)&lightPos, -5.0, 5.0f);
		// set the new position of the light source
		Core::GetInstance()->SetLightPos(lightPos);

		// get number of cubes in the scene
		unsigned int nCubes = Core::GetInstance()->GetNCubes();
		// get a reference to the color of each of the cubes in the scene
		auto& cubeColors = Core::GetInstance()->GetCubeColors();
		// drop-down list of color pickers for each of the cubes in the scene
		for (GLuint i{}; i < nCubes; i++)
		{
			// create a color picker for the i-th cube 
			// (provisionally set the name of the cube to "Cube i", in the future we should be able 
			// to get it from the Cube object itself, since it is an Asset)
			ImGui::ColorEdit3(("Cube " + std::to_string(i) + " Color").c_str(), (float*)&cubeColors[i]);
			// set the new color of the cube
			Core::GetInstance()->SetCubeColor(i, cubeColors[i]);
		}

		ImGui::End();
	}

	// check if ImGui wants to capture the mouse (when interacting with the GUI)
	if (ImGui::GetIO().WantCaptureMouse) // prevent camera manipulation
		Core::GetInstance()->SetCameraControlEnabled(false);
	else // re-enable camera manipulation
		Core::GetInstance()->SetCameraControlEnabled(true);
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