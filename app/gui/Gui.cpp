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

	// set initial window position to the top-left corner (with some padding)
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	// set initial window size to 320x300 pixels
	ImGui::SetNextWindowSize(ImVec2(320, 300), ImGuiCond_FirstUseEver);

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
			ImGui::Text("Color: (%.2f, %.2f, %.2f)", pointLight->GetDiffuse().x, pointLight->GetDiffuse().y, pointLight->GetDiffuse().z);
			// display the position of the point light
			ImGui::Text("Position: (%.2f, %.2f, %.2f)", pointLight->GetPosition().x, pointLight->GetPosition().y, pointLight->GetPosition().z);
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
			ImGui::Text("Shape %d: %s", i, shape->GetName().c_str());
			// display the color of the shape
			ImGui::Text("Color: (%.2f, %.2f, %.2f)", shape->GetAlbedo().x, shape->GetAlbedo().y, shape->GetAlbedo().z);
			// display the position of the shape
			ImGui::Text("Position: (%.2f, %.2f, %.2f)", shape->GetPosition().x, shape->GetPosition().y, shape->GetPosition().z);
		}

		// button to add a new shape to the scene
		if (ImGui::Button("Add Shape"))
		{
			// create a new shape with a name "Shape nShapes" (where nShapes is the current number of shapes in the scene)
			std::string newShapeName = "Shape " + std::to_string(nShapes);
			auto newShape = std::make_unique<Shape>(newShapeName, verticesVec, indicesVec, glm::vec3(0.5f), glm::vec3(0.0f));

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

			// output the new asset to the console
			std::cout << "Asset type: " << "MODEL" << std::endl;
			std::cout << "Asset name: " << newShapeName << std::endl;
		}

		ImGui::End();
	}

	// set initial window position to the top-right corner (with some padding)
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 390, 10), ImGuiCond_FirstUseEver);
	// set initial window size to 380x100 pixels
	ImGui::SetNextWindowSize(ImVec2(380, 100), ImGuiCond_FirstUseEver);

	// show a window that allows the user to change the color of the shapes in the scene and
	// the position of the light source
	{
		ImGui::Begin("Scene Settings");

		// get the light object
		auto light = dynamic_cast<PointLight*>(Core::GetInstance()->GetAssets("LIGHT")[0].get());
		// get the position of the light source
		glm::vec3 lightPos = light->GetPosition();
		// create a slider for the x, y, and z components of the light position
		if (ImGui::SliderFloat3("Light Position", (float*)&lightPos, -5.0, 5.0f))
			light->SetPosition(lightPos); // set the new position of the light source (only if the slider is moved)

		// get number of shapes in the scene
		unsigned int nShapes = Core::GetInstance()->GetNShapes();
		// for each shape in the scene, create a color picker and a slider for its position component
		for (unsigned int i{}; i < nShapes; i++)
		{
			// get the shape object
			auto shape = dynamic_cast<Shape*>(Core::GetInstance()->GetAssets("MODEL")[i].get());
			// get the color of the shape
			glm::vec3 color = shape->GetAlbedo();
			// create a color picker for the shape's color
			if (ImGui::ColorEdit3(("Shape " + std::to_string(i)).c_str(), (float*)&color))
				shape->SetAlbedo(color); // set the new color of the shape (only if the color picker is used)
			// get the position of the shape
			glm::vec3 pos = shape->GetPosition();
			// create a slider for the x, y, and z components of the shape's position
			if (ImGui::SliderFloat3(("Shape " + std::to_string(i) + " Position").c_str(), (float*)&pos, -5.0, 5.0f))
				shape->SetPosition(pos); // set the new position of the shape (only if the slider is moved)
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