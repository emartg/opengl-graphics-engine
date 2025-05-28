/*
* GUI.h
* This file implements the GUI class, which is used to create a graphical user interface
* using the ImGui library.
* The engine will use this class to create a windows to display information about the scene
* and allow the user to interact with it, e.g. change certain parameters or add new objects.
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
	// set initial window size to 320x(DisplaySize.y - 20) pixels 
	// (i.e, full height of the window with some padding)
	ImGui::SetNextWindowSize(ImVec2(320, io.DisplaySize.y - 20), ImGuiCond_Appearing);
	// set the window to be not collapsed (i.e., not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	// show a window that displays all information about the assets in the scene 
	{
		ImGui::Begin("Scene Information");

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
		// get the number of spotlights in the scene
		unsigned int nSpotlights = Core::GetInstance()->GetNSpotlights();
		// display the number of lights in the scene
		ImGui::Text("Number of Lights: %d", nLights);
		// display the number of point lights in the scene
		ImGui::Text("Number of Point Lights: %d", nPointLights);
		// display the number of spotlights in the scene
		ImGui::Text("Number of Spotlights: %d", nSpotlights);

		ImGui::Text("LIGHTS:");
		// for each light in the scene, display its attributes
		for (unsigned int i{}; i < nLights; i++)
		{
			// get the light object and its type
			auto light = dynamic_cast<Light*>(Core::GetInstance()->GetAssets("LIGHT")[i].get());
			LightType lightType = light->GetLightType();

			if (lightType == LightType::POINT_LIGHT) // if the Light is a PointLight
			{
				auto pointLight = dynamic_cast<PointLight*>(light); // get the PointLight object

				// use PushID to create a unique ID for each light (to avoid conflicts with the GUI)
				ImGui::PushID(pointLight->GetName().c_str());

				// display the name of the light
				ImGui::Text("Name: %s", pointLight->GetName().c_str());
				// display the color of the light
				ImGui::Text("Color: (%.2f, %.2f, %.2f)",
							pointLight->GetDiffuse().x, pointLight->GetDiffuse().y, pointLight->GetDiffuse().z);
				// display the position of the light
				ImGui::Text("Position: (%.2f, %.2f, %.2f)",
							pointLight->GetPosition().x, pointLight->GetPosition().y, pointLight->GetPosition().z);

				// use PopID to end the unique ID scope
				ImGui::PopID();
			}
			else if (lightType == LightType::SPOTLIGHT) // if the Light is a Spotlight
			{
				auto spotlight = dynamic_cast<Spotlight*>(light); // get the Spotlight object

				// use PushID to create a unique ID for each spotlight (to avoid conflicts with the GUI)
				ImGui::PushID(spotlight->GetName().c_str());

				// display the name of the spotlight
				ImGui::Text("Name: %s", spotlight->GetName().c_str());
				// display the color of the spotlight
				ImGui::Text("Color: (%.2f, %.2f, %.2f)",
							spotlight->GetDiffuse().x, spotlight->GetDiffuse().y, spotlight->GetDiffuse().z);
				// display the position of the spotlight
				ImGui::Text("Position: (%.2f, %.2f, %.2f)",
							spotlight->GetPosition().x, spotlight->GetPosition().y, spotlight->GetPosition().z);
				// display the direction of the spotlight
				ImGui::Text("Direction: (%.2f, %.2f, %.2f)",
							spotlight->GetDirection().x, spotlight->GetDirection().y, spotlight->GetDirection().z);
				// display the cut-off angles of the spotlight (in degrees)
				ImGui::Text("Inner cut-off: %.2f", glm::degrees(glm::acos(spotlight->GetInnerCutOff())));
				ImGui::Text("Outer cut-off: %.2f", glm::degrees(glm::acos(spotlight->GetOuterCutOff())));

				// use PopID to end the unique ID scope
				ImGui::PopID();
			}
			else
			{
				std::cerr << "Unknown light type: " << light->GetName() << std::endl;
			}
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

			// use PushID to create a unique ID for each shape (to avoid conflicts with the GUI)
			ImGui::PushID(shape->GetName().c_str());

			// display the name of the shape
			ImGui::Text("Name: %s", shape->GetName().c_str());
			// display the color of the shape
			ImGui::Text("Color: (%.2f, %.2f, %.2f)",
						shape->GetAlbedo().x, shape->GetAlbedo().y, shape->GetAlbedo().z);
			// display the position of the shape
			ImGui::Text("Position: (%.2f, %.2f, %.2f)",
						shape->GetPosition().x, shape->GetPosition().y, shape->GetPosition().z);

			// use PopID to end the unique ID scope
			ImGui::PopID();
		}

		ImGui::End();
	}

	// set initial window position to the top-right corner (with some padding)
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 330, 10), ImGuiCond_Appearing);
	// set initial window size to 320x(DisplaySize.y - 20) pixels 
	// (i.e, full height of the window with some padding)
	ImGui::SetNextWindowSize(ImVec2(320, io.DisplaySize.y - 20), ImGuiCond_Appearing);
	// set the window to be not collapsed (i.e., not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	// show a window that allows the user to change the attributes of the lights (and their respective gizmos) 
	// and the shapes in the scene, and create either new light sources (and consequently their gizmos) 
	// or new shapes with random attributes
	{
		ImGui::Begin("Scene Settings");

		// get the number of lights in the scene
		unsigned int nLights = Core::GetInstance()->GetNLights();
		// for each light in the scene, create all the necessary GUI elements to change its attributes
		for (unsigned int i{}; i < nLights; i++)
		{
			// get the light object and its type
			auto light = dynamic_cast<Light*>(Core::GetInstance()->GetAssets("LIGHT")[i].get());
			LightType lightType = light->GetLightType();

			if (lightType == LightType::POINT_LIGHT) // if the Light is a PointLight
			{
				auto pointLight = dynamic_cast<PointLight*>(light); // get the PointLight object

				// use PushID to create a unique ID for each light (to avoid conflicts with the GUI)
				ImGui::PushID(pointLight->GetName().c_str());

				// display the name of the light
				ImGui::Text("%s", pointLight->GetName().c_str());

				// get the color of the light
				glm::vec3 color = pointLight->GetDiffuse();
				// create a color picker for the light's color
				if (ImGui::ColorEdit3("Color", (float*)&color)) // if the color picker is used
				{
					pointLight->SetDiffuse(color); // set the new color of the light
					pointLight->SyncGizmoColorFromLight(); // set the new color of the light's gizmo
				}

				// get the position of the light
				glm::vec3 pos = pointLight->GetPosition();
				// create a slider for the x, y, and z components of the light's position
				if (ImGui::SliderFloat3("Position", (float*)&pos,
										MIN_POSITION_SLIDER_VALUE, MAX_POSITION_SLIDER_VALUE)) // if the slider is moved
				{
					pointLight->SetPosition(pos); // set the new position of the light
					pointLight->SyncGizmoPositionFromLight(); // set the new position of the light's gizmo
				}

				// use PopID to end the unique ID scope
				ImGui::PopID();

				ImGui::Separator(); // add a separator between lights
			}
			else if (lightType == LightType::SPOTLIGHT) // if the Light is a Spotlight
			{
				auto spotlight = dynamic_cast<Spotlight*>(light); // get the Spotlight object

				// use PushID to create a unique ID for each spotlight (to avoid conflicts with the GUI)
				ImGui::PushID(spotlight->GetName().c_str());

				// display the name of the spotlight
				ImGui::Text("%s", spotlight->GetName().c_str());

				// get the color of the spotlight
				glm::vec3 color = spotlight->GetDiffuse();
				// create a color picker for the spotlight's color
				if (ImGui::ColorEdit3("Color", (float*)&color)) // if the color picker is used
				{
					spotlight->SetDiffuse(color); // set the new color of the spotlight
					spotlight->SyncGizmoColorFromLight(); // set the new color of the spotlight's gizmo
				}

				// get the position of the spotlight
				glm::vec3 pos = spotlight->GetPosition();
				// create a slider for the x, y, and z components of the spotlight's position
				if (ImGui::SliderFloat3("Position", (float*)&pos,
										MIN_POSITION_SLIDER_VALUE, MAX_POSITION_SLIDER_VALUE)) // if the slider is moved
				{
					spotlight->SetPosition(pos); // set the new position of the spotlight
					spotlight->SyncGizmoPositionFromLight(); // set the new position of the spotlight's gizmo
				}

				// get the direction of the spotlight
				glm::vec3 dir = spotlight->GetDirection();
				// create a slider for the x, y, and z components of the spotlight's direction
				if (ImGui::SliderFloat3("Direction", (float*)&dir,
										MIN_DIRECTION_SLIDER_VALUE, MAX_DIRECTION_SLIDER_VALUE)) // if the slider is moved
				{
					dir = glm::normalize(dir); // normalize the direction vector to avoid issues with the spotlight's direction
					spotlight->SetDirection(dir); // set the new direction of the spotlight
					spotlight->SyncGizmoDirectionFromLight(); // set the new direction of the spotlight's gizmo
				}

				// get the inner and outer cut-off angles of the spotlight 
				// and convert them from radians (cosine) to degrees for the sliders
				float innerCutOff = glm::degrees(glm::acos(spotlight->GetInnerCutOff()));
				float outerCutOff = glm::degrees(glm::acos(spotlight->GetOuterCutOff()));
				// clamp the maximum value of the inner cut-off angle slider so that it does not exceed the outer cut-off angle
				// (the inner cut-off angle must be less than or equal to the outer cut-off angle)
				float currentInnerMaxCutOffSliderValue = std::min(MAX_INNER_CUTOFF_SLIDER_VALUE, outerCutOff);
				// create a slider for the inner cut-off angle of the spotlight
				if (ImGui::SliderFloat("Inner Cut-off", &innerCutOff,
									   MIN_INNER_CUTOFF_SLIDER_VALUE, currentInnerMaxCutOffSliderValue)) // max is outerCutOff
				{
					// clamp to avoid going above outerCutOff
					// (the inner cut-off angle must be less than or equal to the outer cut-off angle)
					if (innerCutOff > outerCutOff) innerCutOff = outerCutOff;
					// convert back to radians and cosine and set the new inner cut-off angle for the spotlight
					spotlight->SetInnerCutOff(glm::cos(glm::radians(innerCutOff)));
				}
				// create a slider for the outer cut-off angle of the spotlight
				if (ImGui::SliderFloat("Outer Cut-off", &outerCutOff,
									   MIN_OUTER_CUTOFF_SLIDER_VALUE, MAX_OUTER_CUTOFF_SLIDER_VALUE))
				{
					// clamp to avoid going below innerCutOff 
					// (the inner cut-off angle must be less than or equal to the outer cut-off angle)
					if (outerCutOff < innerCutOff) outerCutOff = innerCutOff;
					// convert back to radians and cosine and set the new outer cut-off angle for the spotlight
					spotlight->SetOuterCutOff(glm::cos(glm::radians(outerCutOff)));
				}

				// use PopID to end the unique ID scope
				ImGui::PopID();

				ImGui::Separator(); // add a separator between lights
			}
			else
			{
				std::cerr << "Unknown light type: " << light->GetName() << std::endl;
			}
		}

		// get number of shapes in the scene
		unsigned int nShapes = Core::GetInstance()->GetNShapes();
		// for each shape in the scene, create a color picker and a slider for its position component
		for (unsigned int i{}; i < nShapes; i++)
		{
			// get the shape object
			auto shape = dynamic_cast<Shape*>(Core::GetInstance()->GetAssets("MODEL")[i].get());

			// if the shape is not a gizmo, then proceed, otherwise skip it (as gizmos are already handled in above)
			if (shape->GetGizmoShapeType() == GizmoShapeType::NONE)
			{
				// use PushID to create a unique ID for each shape (to avoid conflicts with the GUI)
				ImGui::PushID(shape->GetName().c_str());

				// display the name of the shape
				ImGui::Text("%s", shape->GetName().c_str());

				// get the color of the shape
				glm::vec3 color = shape->GetAlbedo();
				// create a color picker for the shape's color
				if (ImGui::ColorEdit3("Color", (float*)&color)) // if the color picker is used
					shape->SetAlbedo(color); // set the new color of the shape

				// get the position of the shape
				glm::vec3 pos = shape->GetPosition();
				// create a slider for the x, y, and z components of the shape's position
				if (ImGui::SliderFloat3("Position", (float*)&pos,
										MIN_POSITION_SLIDER_VALUE, MAX_POSITION_SLIDER_VALUE)) // if the slider is moved
				{
					shape->SetPosition(pos); // set the new position of the shape
				}

				// use PopID to end the unique ID scope
				ImGui::PopID();

				ImGui::Separator(); // add a separator between shapes
			}
		}

		ImGui::Separator();

		// button to add a new cube to the scene
		if (ImGui::Button("Add Cube"))
		{
			// get a random color and a random position for the cube
			glm::vec3 newColor = m_randomizer->GenerateRandomColor();
			glm::vec3 newPos = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																	MIN_DISTANCE_FROM_ORIGIN, MAX_DISTANCE_FROM_ORIGIN);

			// get the current number of shapes in the scene
			std::string nShapes = std::to_string(Core::GetInstance()->GetNShapes());
			// create a new cube called "Cube (Shape n)", 
			// where n is the current number of shapes in the scene
			auto newShape = std::make_shared<Shape>("Cube (Shape " + nShapes + ")",
													cubeVerticesVec, cubeIndicesVec,
													newColor, // color (random)
													newPos // position (random)
			);

			// add the new shape (a cube) to the engine
			Core::GetInstance()->AddAsset(std::move(newShape));
		}

		// button to add a new point light to the scene
		if (ImGui::Button("Add Point Light"))
		{
			// get a random color and a random position for the point light and its gizmo
			glm::vec3 newColor = m_randomizer->GenerateRandomColor();
			glm::vec3 newPos = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																	MIN_DISTANCE_FROM_ORIGIN, MAX_DISTANCE_FROM_ORIGIN);

			// get the current number of shapes in the scene
			std::string nShapes = std::to_string(Core::GetInstance()->GetNShapes());
			// get the current number of point lights in the scene
			std::string nPointLights = std::to_string(Core::GetInstance()->GetNPointLights());
			// create a new point light called "Point Light n", 
			// where n is the current number of point lights in the scene
			auto newPointLight = std::make_shared<PointLight>("Point Light " + nPointLights,
															  glm::vec3{ 0.1f }, // ambient color (default)
															  newColor, // diffuse color (random)
															  glm::vec3{ 1.0f }, // specular color (default)
															  newPos // position (random)
			);
			// get gizmo's shared_ptr from the new point light before adding the latter to the engine
			auto newPointLightGizmo = newPointLight->GetGizmoShape();
			Core::GetInstance()->AddAsset(std::move(newPointLight)); // add the new point light to the engine
			// concatenate the name of the new point light and " (Shape n)", 
			// where n is the current number of shapes in the scene
			newPointLightGizmo->SetName(newPointLightGizmo->GetName() + " (Shape " + nShapes + ")");
			// add the point light gizmo (a decahedron) to the engine
			Core::GetInstance()->AddAsset(std::move(newPointLightGizmo));
		}

		// button to add a new spotlight to the scene
		if (ImGui::Button("Add Spotlight"))
		{
			// get a random color, a random position, and a random direction for the spotlight and its gizmo
			glm::vec3 newColor = m_randomizer->GenerateRandomColor();
			glm::vec3 newPos = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																	MIN_DISTANCE_FROM_ORIGIN, MAX_DISTANCE_FROM_ORIGIN);
			glm::vec3 newDir = m_randomizer->GenerateRandomDirection();

			// get the current number of shapes in the scene
			std::string nShapes = std::to_string(Core::GetInstance()->GetNShapes());
			// get the current number of spotlights in the scene
			std::string nSpotlights = std::to_string(Core::GetInstance()->GetNSpotlights());
			// create a new spotlight called "Spotlight n",
			// where n is the current number of spotlights in the scene
			auto newSpotlight = std::make_shared<Spotlight>("Spotlight " + nSpotlights,
															glm::vec3{ 0.1f }, // ambient color (default)
															newColor, // diffuse color (random)
															glm::vec3{ 1.0f }, // specular color (default)
															newPos, // position (random)
															newDir // direction (random)
			);
			// get gizmo's shared_ptr from the new spotlight before adding the latter to the engine
			auto newSpotlightGizmo = newSpotlight->GetGizmoShape();
			Core::GetInstance()->AddAsset(std::move(newSpotlight)); // add the new spotlight to the engine
			// concatenate the name of the new spotlight and " (Shape n)",
			// where n is the current number of shapes in the scene
			newSpotlightGizmo->SetName(newSpotlightGizmo->GetName() + " (Shape " + nShapes + ")");
			// add the spotlight gizmo (a decahedron) to the engine
			Core::GetInstance()->AddAsset(std::move(newSpotlightGizmo));
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