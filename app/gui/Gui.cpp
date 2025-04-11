/*
* GUI.h
* This file implements the GUI class, which is used to create a graphical user interface
* using the ImGui library.
* The engine will use this class to create a window that will display information about the scene
* and allow the user to interact with it and change certain parameters.
*/

#include "Gui.h"
#include "../CUBE.h"
#include "../DECAHEDRON.h"

// Constructors
// ------------
GUI::GUI()
	: m_showDemoWindow{ true }, m_showAnotherWindow{ true },
	m_clearColor{ 0.45f, 0.55f, 0.60f, 1.00f }
{}

// Destructor
// ----------
GUI::~GUI()
{}

// Public Methods
// --------------
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

	// get ImGuiIO object to access the display size later
	ImGuiIO& io = ImGui::GetIO();

	// set initial window position to the top-left corner (with some padding)
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Appearing);
	// set initial window size to 320x(DisplaySize.y - 20) pixels (i.e, full height of the window with some padding)
	ImGui::SetNextWindowSize(ImVec2(320, io.DisplaySize.y - 20), ImGuiCond_Appearing);
	// set the window to be not collapsed (i.e., not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	// show a window that displays all the assets in the scene and allows the user to create a new shape
	{
		ImGui::Begin("Scene Assets");

		// get the camera object
		auto& camera = Core::GetInstance()->GetCamera();
		// display the name of the camera and its position
		ImGui::Text("Camera Name: %s", camera->GetName().c_str());
		ImGui::Text("Camera Position: (%.3f, %.3f, %.3f)",
					camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z);

		ImGui::Separator();

		// get the number of lights in the scene
		unsigned int nLights = Core::GetInstance()->GetNLights();
		// get the number of point lights in the scene
		unsigned int nPointLights = Core::GetInstance()->GetNPointLights();
		// display the number of lights in the scene
		ImGui::Text("Number of Lights: %d", nLights);
		// display the number of point lights in the scene
		ImGui::Text("Number of Point Lights: %d", nPointLights);

		ImGui::Text("POINT LIGHTS:");
		// for each point light in the scene, display its name, color, and position
		for (unsigned int i{}; i < nPointLights; i++)
		{
			// get the point light object
			auto pointLight = dynamic_cast<PointLight*>(Core::GetInstance()->GetAssets("LIGHT")[i].get());
			// display the name of the point light
			ImGui::Text("Point Light %d: %s", i, pointLight->GetName().c_str());
			// display the color of the point light
			ImGui::Text("Color: (%.2f, %.2f, %.2f)",
						pointLight->GetDiffuse().x, pointLight->GetDiffuse().y, pointLight->GetDiffuse().z);
			// display the position of the point light
			ImGui::Text("Position: (%.2f, %.2f, %.2f)",
						pointLight->GetPosition().x, pointLight->GetPosition().y, pointLight->GetPosition().z);
		}

		ImGui::Separator();

		// get the number of models in the scene
		unsigned int nModels = Core::GetInstance()->GetNModels();
		// get the number of shapes in the scene
		unsigned int nShapes = Core::GetInstance()->GetNShapes();
		// display the number of models in the scene
		ImGui::Text("Number of Models: %d", nModels);
		// display the number of shapes in the scene
		ImGui::Text("Number of Shapes: %d", nShapes);

		ImGui::Text("SHAPES:");
		// for each shape in the scene, display its name, color, and position
		for (unsigned int i{}; i < nShapes; i++)
		{
			// get the shape object
			auto shape = dynamic_cast<Shape*>(Core::GetInstance()->GetAssets("MODEL")[i].get());

			// display the name of the shape
			ImGui::Text("Name: %s", shape->GetName().c_str());

			// display the color of the shape
			ImGui::Text("Color: (%.2f, %.2f, %.2f)",
						shape->GetAlbedo().x, shape->GetAlbedo().y, shape->GetAlbedo().z);

			// display the position of the shape
			ImGui::Text("Position: (%.2f, %.2f, %.2f)",
						shape->GetPosition().x, shape->GetPosition().y, shape->GetPosition().z);
		}

		// button to add a new cube to the scene
		if (ImGui::Button("Add Cube"))
		{
			// create a new cube shape called "Cube (Shape n)", (where n is the current number of shapes in the scene)
			std::string newShapeName = "Cube (Shape " + std::to_string(nShapes) + ")";
			auto newShape = std::make_unique<Shape>(newShapeName, cubeVerticesVec, cubeIndicesVec,
													glm::vec3(0.5f), glm::vec3(0.0f));

			// use the current time as seed for the random number generator 
			// (to get different positions and colors each run)
			srand(static_cast<unsigned int>(time(0)));

			// set the shape's color to a random color
			glm::vec3 newColor{ (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f };
			newShape->SetAlbedo(newColor);
			// set the shape's translation randomly 
			// (within a certain range, ensuring it is neither too close to the camera nor too far away)
			glm::vec3 newPos;
			do
			{
				newPos = glm::vec3{ (rand() % 10) - 5, (rand() % 10) - 5, (rand() % 10) - 5 };
			} while (glm::length(newPos - camera->GetPosition()) < 2.0f);
			newShape->SetPosition(newPos);

			// add the new shape to the engine
			Core::GetInstance()->AddAsset(std::move(newShape));
		}

		ImGui::End();
	}

	// set initial window position to the top-right corner (with some padding)
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 330, 10), ImGuiCond_Appearing);
	// set initial window size to 320x(DisplaySize.y - 20) pixels (i.e, full height of the window with some padding)
	ImGui::SetNextWindowSize(ImVec2(320, io.DisplaySize.y - 20), ImGuiCond_Appearing);
	// set the window to be not collapsed (i.e., not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	// show a window that allows the user to change the position and color of the shapes in the scene
	// (some shapes are light sources' gizmos, and modifying their attributes will result in changes in the lighting)
	{
		ImGui::Begin("Scene Settings");

		// get the light object (as there is only one light in the scene in this case)
		auto light = dynamic_cast<PointLight*>(Core::GetInstance()->GetAssets("LIGHT")[0].get());

		// get number of shapes in the scene
		unsigned int nShapes = Core::GetInstance()->GetNShapes();
		// for each shape in the scene, create a color picker and a slider for its position component
		for (unsigned int i{}; i < nShapes; i++)
		{
			// get the shape object
			auto shape = dynamic_cast<Shape*>(Core::GetInstance()->GetAssets("MODEL")[i].get());

			// use PushID to create a unique ID for each shape (to avoid conflicts with the GUI)
			ImGui::PushID(i);

			// display the name of the shape
			ImGui::Text("%s", shape->GetName().c_str());

			// get the color of the shape
			glm::vec3 color = shape->GetAlbedo();
			// create a color picker for the shape's color
			if (ImGui::ColorEdit3("Color", (float*)&color))
			{
				shape->SetAlbedo(color); // set the new color of the shape (only if the color picker is used)
				// if the shape is a point light, also change the color of the light depending on the shape's color
				if (shape->GetName().find("Point Light") != std::string::npos)
				{
					light->SetAmbient(glm::vec3(0.1f) * color); // set the new ambient color of the light
					light->SetDiffuse(color); // set the new diffuse color of the light
					light->SetSpecular(glm::vec3(1.0f) * color); // set the new specular color of the light
				}
			}

			// get the position of the shape
			glm::vec3 pos = shape->GetPosition();
			// create a slider for the x, y, and z components of the shape's position
			if (ImGui::SliderFloat3("Position", (float*)&pos, -5.0, 5.0f))
			{
				shape->SetPosition(pos); // set the new position of the shape (only if the slider is moved)
				// if the shape is a point light, also set the position of the light
				if (shape->GetName().find("Point Light") != std::string::npos)
					light->SetPosition(pos); // set the new position of the light
			}

			// use PopID to end the unique ID scope
			ImGui::PopID();

			// add a separator between shapes
			if (i < nShapes - 1)
				ImGui::Separator();
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

void GUI::Shutdown() const
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}