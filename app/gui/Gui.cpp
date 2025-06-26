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
		// get the number of directional lights in the scene
		unsigned int nDirectionalLights = Core::GetInstance()->GetNDirectionalLights();
		// display the number of lights in the scene
		ImGui::Text("Number of Lights: %d", nLights);
		// display the number of point lights in the scene
		ImGui::Text("Number of Point Lights: %d", nPointLights);
		// display the number of spotlights in the scene
		ImGui::Text("Number of Spotlights: %d", nSpotlights);
		// display the number of directional lights in the scene
		ImGui::Text("Number of Directional Lights: %d", nDirectionalLights);

		ImGui::Text("LIGHTS:");
		// for each light in the scene, display its attributes
		for (unsigned int i{}; i < nLights; i++)
		{
			// get the light object and its type
			auto light = dynamic_cast<Light*>(Core::GetInstance()->GetAssets("LIGHT")[i].get());
			LightType lightType = light->GetLightType();

			switch (lightType) // switch based on the type of the light
			{
				case LightType::DIRECTIONAL_LIGHT: // if the Light is a DirectionalLight
				{
					// get the DirectionalLight object
					auto directionalLight = dynamic_cast<DirectionalLight*>(light);

					// use PushID to create a unique ID for each light (to avoid conflicts with the GUI)
					ImGui::PushID(directionalLight->GetName().c_str());

					// display the name of the light
					ImGui::Text("Name: %s", directionalLight->GetName().c_str());
					// display the color of the light
					ImGui::Text("Color: (%.2f, %.2f, %.2f)",
								directionalLight->GetDiffuse().x, directionalLight->GetDiffuse().y, directionalLight->GetDiffuse().z);
					// display the position of the light (so that the user can see it, but it is not used for directional lights)
					ImGui::Text("Position: (%.2f, %.2f, %.2f)",
								directionalLight->GetPosition().x, directionalLight->GetPosition().y, directionalLight->GetPosition().z);
					// display the direction of the light
					ImGui::Text("Direction: (%.2f, %.2f, %.2f)",
								directionalLight->GetDirection().x, directionalLight->GetDirection().y, directionalLight->GetDirection().z);

					// use PopID to end the unique ID scope
					ImGui::PopID();
				}
				break;
				case LightType::POINT_LIGHT: // if the Light is a PointLight
				{
					// get the PointLight object
					auto pointLight = dynamic_cast<PointLight*>(light);

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
				break;
				case LightType::SPOTLIGHT: // if the Light is a Spotlight
				{
					// get the Spotlight object
					auto spotlight = dynamic_cast<Spotlight*>(light);

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
				break;
				case LightType::UNDEFINED: // if the Light is of an undefined type
					std::cerr << "UNDEFINED light type for light: " << light->GetName() << std::endl;
					return;
				default: // if the Light is of an unknown type
					std::cerr << "Unknown light type for light: " << light->GetName() << std::endl;
					return;
			}
		}



		ImGui::Separator();

		// get the number of models in the scene
		unsigned int nModels = Core::GetInstance()->GetNModels();
		// get the number of shapes in the scene
		unsigned int nShapes = Core::GetInstance()->GetNShapes();
		// get the number of Assimp models in the scene
		unsigned int nAssimpModels = Core::GetInstance()->GetNAssimpModels();
		// display the number of models in the scene
		ImGui::Text("Number of Models: %d", nModels);
		// display the number of shapes in the scene
		ImGui::Text("Number of Shapes: %d", nShapes);
		// display the number of Assimp models in the scene
		ImGui::Text("Number of Assimp models: %d", nAssimpModels);

		ImGui::Text("MODELS:");
		// for each model in the scene, display its name, color, and position
		for (unsigned int i{}; i < nModels; i++)
		{
			// get the model object
			auto model = dynamic_cast<Model*>(Core::GetInstance()->GetAssets("MODEL")[i].get());

			// use PushID to create a unique ID for each model (to avoid conflicts with the GUI)
			ImGui::PushID(model->GetName().c_str());

			// display the name of the model
			ImGui::Text("Name: %s", model->GetName().c_str());
			// display the color of the model
			ImGui::Text("Color: (%.2f, %.2f, %.2f)",
						model->GetAlbedo().x, model->GetAlbedo().y, model->GetAlbedo().z);
			// display the position of the model
			ImGui::Text("Position: (%.2f, %.2f, %.2f)",
						model->GetPosition().x, model->GetPosition().y, model->GetPosition().z);
			ImGui::Text("Rotation: (%.2f, %.2f, %.2f)",
						model->GetRotationInEulerAngles().x, model->GetRotationInEulerAngles().y, model->GetRotationInEulerAngles().z);
			ImGui::Text("Scale: (%.2f, %.2f, %.2f)",
						model->GetScale().x, model->GetScale().y, model->GetScale().z);
			ImGui::Text("Forward: (%.2f, %.2f, %.2f)",
						model->GetForward().x, model->GetForward().y, model->GetForward().z);

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

	// show a window that allows the user to 
	// change the attributes of the lights (and their respective gizmos) and the regular models in the scene, 
	// and create either a new light sources (and their respective gizmos) or a new cube shape with random attributes
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

			switch (lightType) // switch based on the type of the light
			{
				case LightType::DIRECTIONAL_LIGHT: // if the Light is a DirectionalLight
				{
					// get the DirectionalLight object
					auto directionalLight = dynamic_cast<DirectionalLight*>(light);

					// use PushID to create a unique ID for each directional light (to avoid conflicts with the GUI)
					ImGui::PushID(directionalLight->GetName().c_str());

					// display the name of the directional light
					ImGui::Text("%s", directionalLight->GetName().c_str());

					// get the color of the directional light
					glm::vec3 color = directionalLight->GetDiffuse();
					// create a color picker for the directional light's color
					if (ImGui::ColorEdit3("Color", (float*)&color)) // if the color picker is used
					{
						directionalLight->SetDiffuse(color); // set the new color of the directional light
						directionalLight->SyncGizmoColorFromLight(); // set the new color of the directional light's gizmo
					}

					// get the position of the directional light (so that the user can see it, but it is not used for directional lights)
					glm::vec3 pos = directionalLight->GetPosition();
					// create a slider for the x, y, and z components of the directional light's position
					if (ImGui::SliderFloat3("Position", (float*)&pos,
											MIN_POSITION_SLIDER_VALUE, MAX_POSITION_SLIDER_VALUE)) // if the slider is moved
					{
						directionalLight->SetPosition(pos); // set the new position of the directional light
						directionalLight->SyncGizmoPositionFromLight(); // set the new position of the directional light's gizmo
					}

					// get a reference to the gizmo of the directional light 
					// and its rotation in Euler angles
					auto gizmo = directionalLight->GetGizmo();
					glm::vec3 rotDegrees = gizmo->GetRotationInEulerAngles();
					// create a slider for the x, y, and z components of the directionalLight's rotation
					if (ImGui::SliderFloat3("Rotation", (float*)&rotDegrees,
											MIN_ROTATION_SLIDER_VALUE, MAX_ROTATION_SLIDER_VALUE)) // if the slider is moved
					{
						// set the new rotation of the directional light's gizmo
						gizmo->SetRotationInEulerAngles(rotDegrees);
						// update the light's direction based on the gizmo's new forward vector,
						// without causing the gizmo to be re-oriented by SetForward() again
						directionalLight->SetDirectionOnly(gizmo->GetForward());
					}

					// use PopID to end the unique ID scope
					ImGui::PopID();

					ImGui::Separator(); // add a separator between lights
				}
				break;
				case LightType::POINT_LIGHT: // if the Light is a PointLight
				{
					// get the PointLight object
					auto pointLight = dynamic_cast<PointLight*>(light);

					// use PushID to create a unique ID for each point light (to avoid conflicts with the GUI)
					ImGui::PushID(pointLight->GetName().c_str());

					// display the name of the point light
					ImGui::Text("%s", pointLight->GetName().c_str());

					// get the color of the point light
					glm::vec3 color = pointLight->GetDiffuse();
					// create a color picker for the point light's color
					if (ImGui::ColorEdit3("Color", (float*)&color)) // if the color picker is used
					{
						pointLight->SetDiffuse(color); // set the new color of the point light
						pointLight->SyncGizmoColorFromLight(); // set the new color of the point light's gizmo
					}

					// get the position of the point light
					glm::vec3 pos = pointLight->GetPosition();
					// create a slider for the x, y, and z components of the point light's position
					if (ImGui::SliderFloat3("Position", (float*)&pos,
											MIN_POSITION_SLIDER_VALUE, MAX_POSITION_SLIDER_VALUE)) // if the slider is moved
					{
						pointLight->SetPosition(pos); // set the new position of the point light
						pointLight->SyncGizmoPositionFromLight(); // set the new position of the point light's gizmo
					}

					// use PopID to end the unique ID scope
					ImGui::PopID();

					// add a separator between lights
					ImGui::Separator();
				}
				break;
				case LightType::SPOTLIGHT: // if the Light is a Spotlight
				{
					// get the Spotlight object
					auto spotlight = dynamic_cast<Spotlight*>(light);

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

					// get a reference to the gizmo of the spotlight and its rotation in Euler angles
					auto gizmo = spotlight->GetGizmo();
					glm::vec3 rotDegrees = gizmo->GetRotationInEulerAngles();
					// create a slider for the x, y, and z components of the spotlight's rotation
					if (ImGui::SliderFloat3("Rotation", (float*)&rotDegrees,
											MIN_ROTATION_SLIDER_VALUE, MAX_ROTATION_SLIDER_VALUE)) // if the slider is moved
					{
						// set the new rotation of the spotlight's gizmo
						gizmo->SetRotationInEulerAngles(rotDegrees);
						// update the light's direction based on the gizmo's new forward vector,
						// without causing the gizmo to be re-oriented by SetForward() again
						spotlight->SetDirectionOnly(gizmo->GetForward());
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

					// add a separator between lights
					ImGui::Separator();
				}
				break;
				case LightType::UNDEFINED: // if the Light is of an undefined type
					std::cerr << "UNDEFINED light type for light: " << light->GetName() << std::endl;
					return;
				default: // if the Light is of an unknown type
					std::cerr << "Unknown light type for light: " << light->GetName() << std::endl;
					return;
			}
		}

		// get the number of models in the scene
		GLuint nModels = Core::GetInstance()->GetNModels();
		// for each model in the scene, create all the necessary GUI elements to change its attributes
		for (unsigned int i{}; i < nModels; i++)
		{
			// get the model object
			auto model = dynamic_cast<Model*>(Core::GetInstance()->GetAssets("MODEL")[i].get());

			ModelType modelType = model->GetModelType(); // get the type of the model
			GizmoType gizmoType = model->GetGizmoType(); // get the type of the model's gizmo
			// if the model is not a gizmo, then proceed, otherwise skip it (as gizmos are already handled in above)
			if (gizmoType == GizmoType::NONE)
			{
				// use PushID to create a unique ID for each model (to avoid conflicts with the GUI)
				ImGui::PushID(model->GetName().c_str());

				// display the name of the model
				ImGui::Text("%s", model->GetName().c_str());

				// only if the model is a shape, display the color picker
				if (modelType == ModelType::SHAPE)
				{
					// get the color of the model
					glm::vec3 color = model->GetAlbedo();
					// create a color picker for the model's color
					if (ImGui::ColorEdit3("Color", (float*)&color)) // if the color picker is used
					{
						model->SetAlbedo(color); // set the new color of the model
					}
				}

				// get the position of the model
				glm::vec3 pos = model->GetPosition();
				// create a slider for the x, y, and z components of the model's position
				if (ImGui::SliderFloat3("Position", (float*)&pos,
										MIN_POSITION_SLIDER_VALUE, MAX_POSITION_SLIDER_VALUE)) // if the slider is moved
				{
					model->SetPosition(pos); // set the new position of the model
				}

				// get the rotation in Euler angles
				glm::vec3 rotDegrees = model->GetRotationInEulerAngles();
				// create a slider for the x, y, and z components of the model's rotation
				if (ImGui::SliderFloat3("Rotation", (float*)&rotDegrees,
										MIN_ROTATION_SLIDER_VALUE, MAX_ROTATION_SLIDER_VALUE)) // if the slider is moved
				{
					model->SetRotationInEulerAngles(rotDegrees); // set the new rotation of the model
				}

				// get the scale of the model
				glm::vec3 scale = model->GetScale();
				// create a slider for the x, y, and z components of the model's scale
				if (ImGui::SliderFloat3("Scale", (float*)&scale,
										MIN_SCALE_SLIDER_VALUE, MAX_SCALE_SLIDER_VALUE)) // if the slider is moved
				{
					model->SetScale(scale); // set the new scale of the model
				}

				// use PopID to end the unique ID scope
				ImGui::PopID();

				ImGui::Separator(); // add a separator between models
			}
		}

		ImGui::Separator();

		// button to add a new directional light to the scene
		if (ImGui::Button("Add Directional Light"))
		{
			// get a random color and a random direction for the directional light and its gizmo
			glm::vec3 newColor = m_randomizer->GenerateRandomColor();
			glm::vec3 newPos = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																	MIN_DISTANCE_FROM_ORIGIN, MAX_DISTANCE_FROM_ORIGIN);
			glm::vec3 newDir = m_randomizer->GenerateRandomDirection();

			// get the current number of models in the scene
			std::string nModels = std::to_string(Core::GetInstance()->GetNModels());
			// get the current number of directional lights in the scene
			std::string nDirectionalLights = std::to_string(Core::GetInstance()->GetNDirectionalLights());
			// create a new directional light called "Directional Light n", 
			// where n is the current number of directional lights in the scene
			auto newDirectionalLight = std::make_shared<DirectionalLight>("Directional Light " + nDirectionalLights,
																		  glm::vec3{ 0.1f }, // ambient color (default)
																		  newColor, // diffuse color (random)
																		  glm::vec3{ 1.0f }, // specular color (default)
																		  newPos, // position (random)
																		  newDir // direction (random)
			);
			// get gizmo's shared_ptr from the new directional light before adding the latter to the engine
			auto newDirectionalLightGizmo = newDirectionalLight->GetGizmo();
			Core::GetInstance()->AddAsset(std::move(newDirectionalLight)); // add the new directional light to the engine
			// concatenate the name of the new directional light and " (Model n)",
			// where n is the current number of models in the scene
			newDirectionalLightGizmo->SetName(newDirectionalLightGizmo->GetName() + " (Model " + nModels + ")");
			// add the directional light gizmo (a decahedron) to the engine
			Core::GetInstance()->AddAsset(std::move(newDirectionalLightGizmo));
		}

		// button to add a new spotlight to the scene
		if (ImGui::Button("Add Spotlight"))
		{
			// get a random color, a random position, and a random direction for the spotlight and its gizmo
			glm::vec3 newColor = m_randomizer->GenerateRandomColor();
			glm::vec3 newPos = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																	MIN_DISTANCE_FROM_ORIGIN, MAX_DISTANCE_FROM_ORIGIN);
			glm::vec3 newDir = m_randomizer->GenerateRandomDirection();

			// get the current number of models in the scene
			std::string nModels = std::to_string(Core::GetInstance()->GetNModels());
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
			auto newSpotlightGizmo = newSpotlight->GetGizmo();
			Core::GetInstance()->AddAsset(std::move(newSpotlight)); // add the new spotlight to the engine
			// concatenate the name of the new spotlight and " (Model n)", 
			// where n is the current number of models in the scene
			newSpotlightGizmo->SetName(newSpotlightGizmo->GetName() + " (Model " + nModels + ")");
			// add the spotlight gizmo (a decahedron) to the engine
			Core::GetInstance()->AddAsset(std::move(newSpotlightGizmo));
		}

		// button to add a new point light to the scene
		if (ImGui::Button("Add Point Light"))
		{
			// get a random color and a random position for the point light and its gizmo
			glm::vec3 newColor = m_randomizer->GenerateRandomColor();
			glm::vec3 newPos = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																	MIN_DISTANCE_FROM_ORIGIN, MAX_DISTANCE_FROM_ORIGIN);

			// get the current number of models in the scene
			std::string nModels = std::to_string(Core::GetInstance()->GetNModels());
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
			auto newPointLightGizmo = newPointLight->GetGizmo();
			Core::GetInstance()->AddAsset(std::move(newPointLight)); // add the new point light to the engine
			// concatenate the name of the new point light and " (Model n)", 
			// where n is the current number of models in the scene
			newPointLightGizmo->SetName(newPointLightGizmo->GetName() + " (Model " + nModels + ")");
			// add the point light gizmo (a decahedron) to the engine
			Core::GetInstance()->AddAsset(std::move(newPointLightGizmo));
		}

		// buttom to add a new cube shape to the scene
		if (ImGui::Button("Add Cube Shape"))
		{
			// get a random color and a random position for the cube shape
			glm::vec3 newColor = m_randomizer->GenerateRandomColor();
			glm::vec3 newPos = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																	MIN_DISTANCE_FROM_ORIGIN, MAX_DISTANCE_FROM_ORIGIN);
			// get the current number of models in the scene
			std::string nModels = std::to_string(Core::GetInstance()->GetNModels());
			// create a new cube shape called "Cube (Model n)", 
			// where n is the current number of models in the scene
			auto newCubeShape = std::make_shared<Shape>("Cube (Model " + nModels + ")",
														cubeVerticesVec, cubeIndicesVec,
														newColor, // albedo (random)
														newPos // position (random)
			);
			// add the new cube shape to the engine
			Core::GetInstance()->AddAsset(std::move(newCubeShape));
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