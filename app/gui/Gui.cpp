/*
* GUI.h
* This file implements the GUI class, which is used to create a graphical user interface
* using the ImGui library.
* The engine will use this class to create a windows to display information about the scene
* and allow the user to interact with it, e.g. change certain parameters or add new objects.
*/

#include "Gui.h"
#include "ImGuiFileDialog.h"
#include "../CUBE.h"

// Static Attributes
// -----------------
bool GUI::s_proportionalScaling{ true }; // propertional scaling flag is true by default

// Constructors
// ------------
GUI::GUI()
	: m_clearColor{ 0.45f, 0.55f, 0.60f, 1.00f }, // set the clear color to a light gray by default
	m_randomizer{ std::make_unique<Random>() } // create a random number generator
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

	// Configuration of the GUI
	// ------------------------
	// relative width of a panel window (30% of the display width)
	float panelRelativeWidth{ 0.3f };
	// padding for the panel window (position and size)
	ImVec2 panelPadding{ 20.0f, 10.0f };
	// relative offset for the right panel window (75% of the display width)
	float rightPanelOffset{ 0.75f };

	// size (width and height) of a panel window (30% of the display width, full height minus padding)
	ImVec2 panelSize{ io.DisplaySize.x * panelRelativeWidth, io.DisplaySize.y - panelPadding.x };
	// position of the left panel window (top left corner with padding)
	ImVec2 leftPanelPosition{ panelPadding.y, panelPadding.y };
	// position of the right panel window (top right corner with padding)
	ImVec2 rightPanelPosition{ io.DisplaySize.x * rightPanelOffset - panelPadding.y, panelPadding.y };

	// Scene Information Panel Window
	// ------------------------------
	// set initial size and position for the left panel window
	ImGui::SetNextWindowSize(panelSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(leftPanelPosition, ImGuiCond_Appearing);
	// set the left panel window to be collapsed (i.e. minimized)
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_Appearing);

	// show a window that displays all information about the assets in the scene 
	{
		ImGui::Begin("Scene Information");

		// get the camera object
		auto& camera = Core::GetInstance()->GetCamera();
		// display the name of the camera and its position
		ImGui::Text("Camera Name: %s", camera->GetName().c_str());
		ImGui::Text("Camera Position: (%.3f, %.3f, %.3f)",
					camera->GetPosition().x,
					camera->GetPosition().y,
					camera->GetPosition().z);

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

					// use PushID to create a unique ID for each light 
					ImGui::PushID(directionalLight->GetName().c_str());

					// display the name of the light
					ImGui::Text("Name: %s", directionalLight->GetName().c_str());
					// display the color of the light
					ImGui::Text("Color: (%.3f, %.3f, %.3f)",
								directionalLight->GetDiffuse().x,
								directionalLight->GetDiffuse().y,
								directionalLight->GetDiffuse().z);
					// display the position of the light 
					// (so that the user can see it, but it is not used for directional lights)
					ImGui::Text("Position: (%.3f, %.3f, %.3f)",
								directionalLight->GetPosition().x,
								directionalLight->GetPosition().y,
								directionalLight->GetPosition().z);
					// display the direction of the light
					ImGui::Text("Direction: (%.3f, %.3f, %.3f)",
								directionalLight->GetDirection().x,
								directionalLight->GetDirection().y,
								directionalLight->GetDirection().z);


					ImGui::PopID(); // use PopID to end the unique ID scope
				}
				break;
				case LightType::POINT_LIGHT: // if the Light is a PointLight
				{
					// get the PointLight object
					auto pointLight = dynamic_cast<PointLight*>(light);

					// use PushID to create a unique ID for each light 
					ImGui::PushID(pointLight->GetName().c_str());

					// display the name of the light
					ImGui::Text("Name: %s", pointLight->GetName().c_str());
					// display the color of the light
					ImGui::Text("Color: (%.3f, %.3f, %.3f)",
								pointLight->GetDiffuse().x,
								pointLight->GetDiffuse().y,
								pointLight->GetDiffuse().z);
					// display the position of the light
					ImGui::Text("Position: (%.3f, %.3f, %.3f)",
								pointLight->GetPosition().x,
								pointLight->GetPosition().y,
								pointLight->GetPosition().z);


					ImGui::PopID(); // use PopID to end the unique ID scope
				}
				break;
				case LightType::SPOTLIGHT: // if the Light is a Spotlight
				{
					// get the Spotlight object
					auto spotlight = dynamic_cast<Spotlight*>(light);

					// use PushID to create a unique ID for each spotlight 
					ImGui::PushID(spotlight->GetName().c_str());

					// display the name of the spotlight
					ImGui::Text("Name: %s", spotlight->GetName().c_str());
					// display the color of the spotlight
					ImGui::Text("Color: (%.3f, %.3f, %.3f)",
								spotlight->GetDiffuse().x,
								spotlight->GetDiffuse().y,
								spotlight->GetDiffuse().z);
					// display the position of the spotlight
					ImGui::Text("Position: (%.3f, %.3f, %.3f)",
								spotlight->GetPosition().x,
								spotlight->GetPosition().y,
								spotlight->GetPosition().z);
					// display the direction of the spotlight
					ImGui::Text("Direction: (%.3f, %.3f, %.3f)",
								spotlight->GetDirection().x,
								spotlight->GetDirection().y,
								spotlight->GetDirection().z);
					// display the cut-off angles of the spotlight (in degrees)
					ImGui::Text("Inner cut-off: %.3f", glm::degrees(glm::acos(spotlight->GetInnerCutOff())));
					ImGui::Text("Outer cut-off: %.3f", glm::degrees(glm::acos(spotlight->GetOuterCutOff())));


					ImGui::PopID(); // use PopID to end the unique ID scope
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

			// use PushID to create a unique ID for each model 
			ImGui::PushID(model->GetName().c_str());

			// display the name of the model
			ImGui::Text("Name: %s", model->GetName().c_str());
			// display the color of the model
			ImGui::Text("Color: (%.3f, %.3f, %.3f)",
						model->GetAlbedo().x,
						model->GetAlbedo().y,
						model->GetAlbedo().z);
			// display the position of the model
			ImGui::Text("Position: (%.3f, %.3f, %.3f)",
						model->GetPosition().x,
						model->GetPosition().y,
						model->GetPosition().z);
			ImGui::Text("Rotation: (%.3f, %.3f, %.3f)",
						model->GetRotationInEulerAngles().x,
						model->GetRotationInEulerAngles().y,
						model->GetRotationInEulerAngles().z);
			ImGui::Text("Scale: (%.3f, %.3f, %.3f)",
						model->GetScale().x,
						model->GetScale().y,
						model->GetScale().z);
			ImGui::Text("Forward: (%.3f, %.3f, %.3f)",
						model->GetForward().x,
						model->GetForward().y,
						model->GetForward().z);

			ImGui::PopID(); // use PopID to end the unique ID scope
		}

		ImGui::End();
	}

	// Scene Settings Panel Window
	// ---------------------------
	// set initial size and position for the right panel window
	ImGui::SetNextWindowSize(panelSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(rightPanelPosition, ImGuiCond_Appearing);
	// set the right panel window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	// show a window that allows the user to change the properties of the assets in the scene
	// and add new assets to the scene
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

					// use PushID to create a unique ID for each directional light 
					ImGui::PushID(directionalLight->GetName().c_str());

					// display the name of the directional light
					ImGui::Text("%s", directionalLight->GetName().c_str());

					// get the color of the directional light
					glm::vec3 color = directionalLight->GetDiffuse();
					// create a color picker for the directional light's color
					if (ImGui::ColorEdit3("Color", (float*)&color)) // if the color picker is used
					{
						directionalLight->SetDiffuse(color); // set the new color of the directional light
						directionalLight->SyncGizmoColorFromLight(); // set the new color of the gizmo
					}

					// get the position of the directional light 
					// (so that the user can see it, but it is not used for directional lights)
					glm::vec3 pos = directionalLight->GetPosition();
					// create a slider for the x, y, and z components of the directional light's position
					if (ImGui::SliderFloat3("Position", (float*)&pos,
											MIN_POSITION_SLIDER_VALUE,
											MAX_POSITION_SLIDER_VALUE)) // if the slider is moved
					{
						directionalLight->SetPosition(pos); // set the new position of the directional light
						directionalLight->SyncGizmoPositionFromLight(); // set the new position of the gizmo
					}

					// get a reference to the gizmo of the directional light 
					// and its rotation in Euler angles
					auto gizmo = directionalLight->GetGizmo();
					glm::vec3 rotDegrees = gizmo->GetRotationInEulerAngles();
					// create a slider for the x, y, and z components of the directionalLight's rotation
					if (ImGui::SliderFloat3("Rotation", (float*)&rotDegrees,
											MIN_ROTATION_SLIDER_VALUE,
											MAX_ROTATION_SLIDER_VALUE)) // if the slider is moved
					{
						// set the new rotation of the directional light's gizmo
						gizmo->SetRotationInEulerAngles(rotDegrees);
						// update the light's direction based on the gizmo's new forward vector,
						// without causing the gizmo to be re-oriented by SetForward() again
						directionalLight->SetDirectionOnly(gizmo->GetForward());
					}


					ImGui::PopID(); // use PopID to end the unique ID scope

					ImGui::Separator(); // add a separator between lights
				}
				break;
				case LightType::POINT_LIGHT: // if the Light is a PointLight
				{
					// get the PointLight object
					auto pointLight = dynamic_cast<PointLight*>(light);

					// use PushID to create a unique ID for each point light 
					ImGui::PushID(pointLight->GetName().c_str());

					// display the name of the point light
					ImGui::Text("%s", pointLight->GetName().c_str());

					// get the color of the point light
					glm::vec3 color = pointLight->GetDiffuse();
					// create a color picker for the point light's color
					if (ImGui::ColorEdit3("Color", (float*)&color)) // if the color picker is used
					{
						pointLight->SetDiffuse(color); // set the new color of the point light
						pointLight->SyncGizmoColorFromLight(); // set the new color of the gizmo
					}

					// get the position of the point light
					glm::vec3 pos = pointLight->GetPosition();
					// create a slider for the x, y, and z components of the point light's position
					if (ImGui::SliderFloat3("Position", (float*)&pos,
											MIN_POSITION_SLIDER_VALUE,
											MAX_POSITION_SLIDER_VALUE)) // if the slider is moved
					{
						pointLight->SetPosition(pos); // set the new position of the point light
						pointLight->SyncGizmoPositionFromLight(); // set the new position of the gizmo
					}

					ImGui::PopID(); // use PopID to end the unique ID scope

					ImGui::Separator(); // add a separator between lights
				}
				break;
				case LightType::SPOTLIGHT: // if the Light is a Spotlight
				{
					// get the Spotlight object
					auto spotlight = dynamic_cast<Spotlight*>(light);

					// use PushID to create a unique ID for each spotlight 
					ImGui::PushID(spotlight->GetName().c_str());

					// display the name of the spotlight
					ImGui::Text("%s", spotlight->GetName().c_str());

					// get the color of the spotlight
					glm::vec3 color = spotlight->GetDiffuse();
					// create a color picker for the spotlight's color
					if (ImGui::ColorEdit3("Color", (float*)&color)) // if the color picker is used
					{
						spotlight->SetDiffuse(color); // set the new color of the spotlight
						spotlight->SyncGizmoColorFromLight(); // set the new color of the gizmo
					}

					// get the position of the spotlight
					glm::vec3 pos = spotlight->GetPosition();
					// create a slider for the x, y, and z components of the spotlight's position
					if (ImGui::SliderFloat3("Position", (float*)&pos,
											MIN_POSITION_SLIDER_VALUE,
											MAX_POSITION_SLIDER_VALUE)) // if the slider is moved
					{
						spotlight->SetPosition(pos); // set the new position of the spotlight
						spotlight->SyncGizmoPositionFromLight(); // set the new position of the gizmo
					}

					// get a reference to the gizmo of the spotlight and its rotation in Euler angles
					auto gizmo = spotlight->GetGizmo();
					glm::vec3 rotDegrees = gizmo->GetRotationInEulerAngles();
					// create a slider for the x, y, and z components of the spotlight's rotation
					if (ImGui::SliderFloat3("Rotation", (float*)&rotDegrees,
											MIN_ROTATION_SLIDER_VALUE,
											MAX_ROTATION_SLIDER_VALUE)) // if the slider is moved
					{
						// set the new rotation of the spotlight's gizmo
						gizmo->SetRotationInEulerAngles(rotDegrees);
						// update the light's direction based on the gizmo's new forward vector,
						// without causing the gizmo to be re-oriented by SetForward() again
						spotlight->SetDirectionOnly(gizmo->GetForward());
					}

					// get the inner and outer cut-off angles of the spotlight and 
					// convert them from radians (cosine) to degrees for the sliders
					float innerCutOff = glm::degrees(glm::acos(spotlight->GetInnerCutOff()));
					float outerCutOff = glm::degrees(glm::acos(spotlight->GetOuterCutOff()));
					// clamp the maximum value of the inner cut-off angle slider 
					// so that it does not exceed the outer cut-off angle
					// (the inner cut-off angle must be less than or equal to the outer cut-off angle)
					float currentInnerMaxCutOffSliderValue = std::min(MAX_INNER_CUTOFF_SLIDER_VALUE,
																	  outerCutOff);
					// create a slider for the inner cut-off angle of the spotlight
					if (ImGui::SliderFloat("Inner Cut-off", &innerCutOff,
										   MIN_INNER_CUTOFF_SLIDER_VALUE,
										   currentInnerMaxCutOffSliderValue)) // max is outerCutOff
					{
						// clamp to avoid going above outerCutOff
						// (the inner cut-off angle must be less than or equal to the outer cut-off angle)
						if (innerCutOff > outerCutOff) innerCutOff = outerCutOff;
						// convert back to radians and cosine and 
						// set the new inner cut-off angle for the spotlight
						spotlight->SetInnerCutOff(glm::cos(glm::radians(innerCutOff)));
					}
					// create a slider for the outer cut-off angle of the spotlight
					if (ImGui::SliderFloat("Outer Cut-off", &outerCutOff,
										   MIN_OUTER_CUTOFF_SLIDER_VALUE,
										   MAX_OUTER_CUTOFF_SLIDER_VALUE))
					{
						// clamp to avoid going below innerCutOff 
						// (the inner cut-off angle must be less than or equal to the outer cut-off angle)
						if (outerCutOff < innerCutOff) outerCutOff = innerCutOff;
						// convert back to radians and cosine and 
						// set the new outer cut-off angle for the spotlight
						spotlight->SetOuterCutOff(glm::cos(glm::radians(outerCutOff)));
					}

					ImGui::PopID(); // use PopID to end the unique ID scope

					ImGui::Separator(); // add a separator between lights
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
			// if the model is not a gizmo, then proceed, otherwise skip it (gizmos already handled above)
			if (gizmoType == GizmoType::NONE)
			{
				// use PushID to create a unique ID for each model 
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
										MIN_POSITION_SLIDER_VALUE,
										MAX_POSITION_SLIDER_VALUE)) // if the slider is moved
				{
					model->SetPosition(pos); // set the new position of the model
				}

				// get the rotation in Euler angles
				glm::vec3 rotDegrees = model->GetRotationInEulerAngles();
				// create a slider for the x, y, and z components of the model's rotation
				if (ImGui::SliderFloat3("Rotation", (float*)&rotDegrees,
										MIN_ROTATION_SLIDER_VALUE,
										MAX_ROTATION_SLIDER_VALUE)) // if the slider is moved
				{
					model->SetRotationInEulerAngles(rotDegrees); // set the new rotation of the model
				}

				// get the scale of the model
				glm::vec3 scale = model->GetScale();
				// get name of the model to use it for unique IDs
				std::string name = model->GetName();

				ImGui::Text("Scale");

				// checkbox to enable/disable proportional scaling
				ImGui::Checkbox("Proportional Scaling", &s_proportionalScaling);

				// editable fields with up/down arrows for each component
				float prevScaleX = scale.x, prevScaleY = scale.y, prevScaleZ = scale.z;
				float step = 0.001f; // step size for arrows

				// use PushID to create a unique ID for the scale controls
				ImGui::PushID(name.append("_scale").c_str());

				// X
				ImGui::PushID(name.append("_scaleX").c_str());
				ImGui::SetNextItemWidth(60);
				ImGui::InputFloat("x", &scale.x, 0.0f, 0.0f, "%.3f");
				ImGui::SameLine();
				if (ImGui::ArrowButton("##upX", ImGuiDir_Up)) scale.x += step;
				ImGui::SameLine();
				if (ImGui::ArrowButton("##downX", ImGuiDir_Down)) scale.x -= step;
				ImGui::PopID(); // use PopID to end the unique ID scope

				// Y
				ImGui::PushID(name.append("_scaleY").c_str());
				ImGui::SetNextItemWidth(60);
				ImGui::InputFloat("y", &scale.y, 0.0f, 0.0f, "%.3f");
				ImGui::SameLine();
				if (ImGui::ArrowButton("##upY", ImGuiDir_Up)) scale.y += step;
				ImGui::SameLine();
				if (ImGui::ArrowButton("##downY", ImGuiDir_Down)) scale.y -= step;
				ImGui::PopID(); // use PopID to end the unique ID scope

				// Z
				ImGui::PushID(name.append("_scaleZ").c_str());
				ImGui::SetNextItemWidth(60);
				ImGui::InputFloat("z", &scale.z, 0.0f, 0.0f, "%.3f");
				ImGui::SameLine();
				if (ImGui::ArrowButton("##upZ", ImGuiDir_Up)) scale.z += step;
				ImGui::SameLine();
				if (ImGui::ArrowButton("##downZ", ImGuiDir_Down)) scale.z -= step;
				ImGui::PopID(); // use PopID to end the unique ID scope

				ImGui::PopID(); // end of unique ID scope for scale controls

				// ensure scale values are within the defined limits
				scale.x = std::clamp(scale.x, MIN_SCALE_SLIDER_VALUE, MAX_SCALE_SLIDER_VALUE);
				scale.y = std::clamp(scale.y, MIN_SCALE_SLIDER_VALUE, MAX_SCALE_SLIDER_VALUE);
				scale.z = std::clamp(scale.z, MIN_SCALE_SLIDER_VALUE, MAX_SCALE_SLIDER_VALUE);

				// if proportional scaling is enabled, 
				// set the other components to the same value as the one that was changed by the user
				if (s_proportionalScaling)
				{
					// if the user changes the x component, set y and z to the same value
					if (scale.x != prevScaleX)
					{
						scale.y = scale.x;
						scale.z = scale.x;
					}
					// if the user changes the y component, set x and z to the same value
					else if (scale.y != prevScaleY)
					{
						scale.x = scale.y;
						scale.z = scale.y;
					}
					// if the user changes the z component, set x and y to the same value
					else if (scale.z != prevScaleZ)
					{
						scale.x = scale.z;
						scale.y = scale.z;
					}
				}

				// apply the new scale to the model only if it has changed
				if (scale.x != prevScaleX || scale.y != prevScaleY || scale.z != prevScaleZ)
					model->SetScale(scale);

				ImGui::PopID(); // use PopID to end the unique ID scope 

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
																	MIN_DISTANCE_FROM_ORIGIN,
																	MAX_DISTANCE_FROM_ORIGIN);
			glm::vec3 newDir = m_randomizer->GenerateRandomDirection();

			// get the current number of models in the scene
			std::string nModels = std::to_string(Core::GetInstance()->GetNModels());
			// get the current number of directional lights in the scene
			std::string nDirectionalLights = std::to_string(Core::GetInstance()->GetNDirectionalLights());
			// create a new directional light called "Directional Light n", 
			// where n is the current number of directional lights in the scene
			auto newDirectionalLight = std::make_shared<DirectionalLight>(
				"Directional Light " + nDirectionalLights,
				glm::vec3{ 0.1f }, // ambient color (default)
				newColor, // diffuse color (random)
				glm::vec3{ 1.0f }, // specular color (default)
				newPos, // position (random)
				newDir // direction (random)
			);
			// get gizmo's shared_ptr from the new directional light before adding the latter to the engine
			auto newDirectionalLightGizmo = newDirectionalLight->GetGizmo();
			// add the new directional light to the engine
			Core::GetInstance()->AddAsset(std::move(newDirectionalLight));
			// concatenate the name of the new directional light and " (Model n)",
			// where n is the current number of models in the scene
			newDirectionalLightGizmo->SetName(newDirectionalLightGizmo->GetName()
											  + " (Model " + nModels + ")");
			// add the directional light gizmo (a decahedron) to the engine
			Core::GetInstance()->AddAsset(std::move(newDirectionalLightGizmo));
		}

		// button to add a new spotlight to the scene
		if (ImGui::Button("Add Spotlight"))
		{
			// get a random color, a random position, and a random direction for the spotlight and its gizmo
			glm::vec3 newColor = m_randomizer->GenerateRandomColor();
			glm::vec3 newPos = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																	MIN_DISTANCE_FROM_ORIGIN,
																	MAX_DISTANCE_FROM_ORIGIN);
			glm::vec3 newDir = m_randomizer->GenerateRandomDirection();

			// get the current number of models in the scene
			std::string nModels = std::to_string(Core::GetInstance()->GetNModels());
			// get the current number of spotlights in the scene
			std::string nSpotlights = std::to_string(Core::GetInstance()->GetNSpotlights());
			// create a new spotlight called "Spotlight n", 
			// where n is the current number of spotlights in the scene
			auto newSpotlight = std::make_shared<Spotlight>(
				"Spotlight " + nSpotlights,
				glm::vec3{ 0.1f }, // ambient color (default)
				newColor, // diffuse color (random)
				glm::vec3{ 1.0f }, // specular color (default)
				newPos, // position (random)
				newDir // direction (random)
			);
			// get gizmo's shared_ptr from the new spotlight before adding the latter to the engine
			auto newSpotlightGizmo = newSpotlight->GetGizmo();
			// add the new spotlight to the engine
			Core::GetInstance()->AddAsset(std::move(newSpotlight));
			// concatenate the name of the new spotlight and " (Model n)", 
			// where n is the current number of models in the scene
			newSpotlightGizmo->SetName(newSpotlightGizmo->GetName()
									   + " (Model " + nModels + ")");
			// add the spotlight gizmo (a decahedron) to the engine
			Core::GetInstance()->AddAsset(std::move(newSpotlightGizmo));
		}

		// button to add a new point light to the scene
		if (ImGui::Button("Add Point Light"))
		{
			// get a random color and a random position for the point light and its gizmo
			glm::vec3 newColor = m_randomizer->GenerateRandomColor();
			glm::vec3 newPos = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																	MIN_DISTANCE_FROM_ORIGIN,
																	MAX_DISTANCE_FROM_ORIGIN);

			// get the current number of models in the scene
			std::string nModels = std::to_string(Core::GetInstance()->GetNModels());
			// get the current number of point lights in the scene
			std::string nPointLights = std::to_string(Core::GetInstance()->GetNPointLights());
			// create a new point light called "Point Light n", 
			// where n is the current number of point lights in the scene
			auto newPointLight = std::make_shared<PointLight>(
				"Point Light " + nPointLights,
				glm::vec3{ 0.1f }, // ambient color (default)
				newColor, // diffuse color (random)
				glm::vec3{ 1.0f }, // specular color (default)
				newPos // position (random)
			);
			// get gizmo's shared_ptr from the new point light before adding the latter to the engine
			auto newPointLightGizmo = newPointLight->GetGizmo();
			// add the new point light to the engine
			Core::GetInstance()->AddAsset(std::move(newPointLight));
			// concatenate the name of the new point light and " (Model n)", 
			// where n is the current number of models in the scene
			newPointLightGizmo->SetName(newPointLightGizmo->GetName()
										+ " (Model " + nModels + ")");
			// add the point light gizmo (a decahedron) to the engine
			Core::GetInstance()->AddAsset(std::move(newPointLightGizmo));
		}

		// buttom to add a new cube shape to the scene
		if (ImGui::Button("Add Cube Shape"))
		{
			// get a random color and a random position for the cube shape
			glm::vec3 newColor = m_randomizer->GenerateRandomColor();
			glm::vec3 newPos = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																	MIN_DISTANCE_FROM_ORIGIN,
																	MAX_DISTANCE_FROM_ORIGIN);
			// get the current number of models in the scene
			std::string nModels = std::to_string(Core::GetInstance()->GetNModels());
			// create a new cube shape called "Cube (Model n)", 
			// where n is the current number of models in the scene
			auto newCubeShape = std::make_shared<Shape>(
				"Cube (Model " + nModels + ")",
				cubeVerticesVec, cubeIndicesVec,
				newColor, // albedo (random)
				newPos // position (random)
			);
			// add the new cube shape to the engine
			Core::GetInstance()->AddAsset(std::move(newCubeShape));
		}

		// button to import a new model from a file
		if (ImGui::Button("Import 3D Model"))
		{
			// file dialog configuration
			IGFD::FileDialogConfig fileDialogConfig;
			fileDialogConfig.path = "."; // initial directory to open the file dialog
			fileDialogConfig.countSelectionMax = 1; // for now, allow only one file to be selected
			fileDialogConfig.flags = ImGuiFileDialogFlags_None; // no special flags for the file dialog

			// open a file dialog to select a model file
			ImGuiFileDialog::Instance()->OpenDialog(
				"ChooseFileDlgKey", // unique key for the file dialog
				"Choose Model File", // title of the file dialog
				".obj, .fbx, .dae, .gltf, .glb, .stl, .ply, .3ds", // supported file extensions
				fileDialogConfig // file dialog configuration
			);
		}

		// set the initial window size to 1000 x 600 pixels
		ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_Appearing);
		// set the initial window position to the center of the screen
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x / 2 - 500, io.DisplaySize.y / 2 - 300),
								ImGuiCond_Appearing);
		// set the window to be not collapsed (i.e. not minimized)
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

		// check if the file dialog is displayed and if the user selected a file
		if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
		{
			if (ImGuiFileDialog::Instance()->IsOk()) // if the user selected a file
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

				// normalize slashes to forward slashes for cross-platform texture loading
				std::replace(filePathName.begin(), filePathName.end(), '\\', '/');

				// get the current number of models in the scene
				std::string nModels = std::to_string(Core::GetInstance()->GetNModels());
				// create a new model from the selected file
				auto newAssimpModel = std::make_shared<AssimpModel>(
					fileName + " (Model " + nModels + ")",
					filePathName
				);
				// add the new model to the engine
				Core::GetInstance()->AddAsset(std::move(newAssimpModel));
			}

			ImGuiFileDialog::Instance()->Close(); // close the file dialog
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