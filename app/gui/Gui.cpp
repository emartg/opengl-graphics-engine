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
	m_randomizer{ std::make_unique<Random>() }, // create a random number generator
	m_newAlbedo{ 0.8f }, // default albedo color for new objects is light gray
	m_newPosition{ 0.0f }, // default position for new objects is the origin
	m_newDirection{ 0.0f, 0.0f, -1.0f }, // default direction for new objects is negative z-axis
	m_newInnerCutOff{ 12.5f }, // default inner cutoff angle for new spotlights
	m_newOuterCutOff{ 32.5f } // default outer cutoff angle for new spotlights
{
	initGUILayoutAttributes(); // initialize the screen size-independent layout attributes
	// initialize the positions and sizes of the GUI windows to default values
	// (since they require the ImGui context to be created first to access the ImGui IO object)
	m_informationWindowPosition, m_informationWindowPosition, m_addObjectWindowPosition =
		ImVec2{ 0.0f, 0.0f };
	m_informationWindowSize, m_settingsWindowSize, m_addObjectWindowSize =
		ImVec2{ 0.0f, 0.0f };
}

// Destructor
// ----------
GUI::~GUI()
{}

// Public Methods
// --------------
void GUI::InitGUI(GLFWwindow* window, const char* glslVersion)
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

void GUI::BuildGUI()
{
	beginGUIFrame(); // start a new ImGui frame
	configureGUIStyle(); // configure the ImGui style (optional, can be customized)
	configureGUILayout(); // configure the layout of the GUI windows based on the display size
	drawGUIWindows(); // draw the GUI windows (information, settings, and add object windows)
	handleImGuiInput(); // handle ImGui input (mouse and keyboard)
}

void GUI::RenderGUI()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::ShutdownGUI() const
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

// Private methods
// ---------------
void GUI::initGUILayoutAttributes()
{
	// set the private variables for the GUI layout
	m_informationWindowRelativeWidth = 0.3f;
	m_informationWindowRelativeHeight = 1.0f;
	m_settingsWindowRelativeWidth = 0.2f;
	m_settingsWindowRelativeHeight = 0.8f;
	m_addObjectWindowRelativeWidth = 0.2f;
	m_addObjectWindowRelativeHeight = 0.2f;

	m_informationWindowXOffset = 0.0f;
	m_informationWindowYOffset = 0.0f;
	m_settingsWindowXOffset = 1.0f - m_settingsWindowRelativeWidth;
	m_settingsWindowYOffset = 0.0f;
	m_addObjectWindowXOffset = 1.0f - m_addObjectWindowRelativeWidth;
	m_addObjectWindowYOffset = 1.0f - m_addObjectWindowRelativeHeight;
	m_windowPositionPadding = ImVec2{ 10.f, 10.0f };
	m_windowSizePadding = ImVec2{ 20.f, 20.0f };
}

void GUI::beginGUIFrame() const
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}
void GUI::configureGUIStyle() const
{

}
void GUI::configureGUILayout()
{
	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for font and style settings

	// position of the information window (top left corner with padding)
	m_informationWindowPosition = ImVec2{
		io.DisplaySize.x * m_informationWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_informationWindowYOffset + m_windowPositionPadding.y // y position
	};
	// position of the settings window (top right corner with padding)
	m_settingsWindowPosition = ImVec2{
		io.DisplaySize.x * m_settingsWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_settingsWindowYOffset + m_windowPositionPadding.y // y position
	};
	// position of the add object window (bottom right corner with padding)
	m_addObjectWindowPosition = ImVec2{
		io.DisplaySize.x * m_addObjectWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_addObjectWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the information window
	m_informationWindowSize = ImVec2{
		io.DisplaySize.x * m_informationWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_informationWindowRelativeHeight - m_windowSizePadding.y // height
	};
	// size (width and height) of the settings window
	m_settingsWindowSize = ImVec2{
		io.DisplaySize.x * m_settingsWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_settingsWindowRelativeHeight - m_windowSizePadding.y * 0.5f // height
	};
	// size (width and height) of the add object window
	m_addObjectWindowSize = ImVec2{
		io.DisplaySize.x * m_addObjectWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_addObjectWindowRelativeHeight - m_windowSizePadding.y // height
	};
}
void GUI::drawGUIWindows()
{
	// draw the information window
	drawSceneInformationWindow();
	// draw the settings window
	drawSceneSettingsWindow();
	// draw the add object window
	drawAddObjectWindow();
}
void GUI::handleImGuiInput() const
{
	// get the input manager from the Core instance
	auto& inputManager = Core::GetInstance()->GetInputManager();

	// check if ImGui wants to capture the mouse (when interacting with the GUI)
	if (ImGui::GetIO().WantCaptureMouse) // prevent camera manipulation
		inputManager->SetCameraControlEnabled(false);
	else // re-enable camera manipulation
		inputManager->SetCameraControlEnabled(true);
}

void GUI::drawSceneInformationWindow() const
{
	// set initial size and position for the information window
	ImGui::SetNextWindowSize(m_informationWindowSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(m_informationWindowPosition, ImGuiCond_Appearing);
	// set the information window to be collapsed (i.e. minimized)
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_Appearing);

	{ // show a window that displays all information about the assets in the scene
		ImGui::Begin("Scene Information");

		drawCamerasInformation(); // display information about the cameras in the scene
		ImGui::Separator();
		drawLightsInformation(); // display information about the lights in the scene
		ImGui::Separator();
		drawModelsInformation(); // display information about the models in the scene

		ImGui::End(); // end the Scene Information window
	}
}
void GUI::drawSceneSettingsWindow()
{
	// get the asset manager, the input manager, and the scene manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	// set initial size and position for the settings window
	ImGui::SetNextWindowSize(m_settingsWindowSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(m_settingsWindowPosition, ImGuiCond_Appearing);
	// set the settings window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that allows the user to change the properties of the assets in the scene
		ImGui::Begin("Scene Settings");

		std::for_each(assetManager->GetAssets("LIGHT").begin(),
					  assetManager->GetAssets("LIGHT").end(),
					  [&](const std::shared_ptr<Asset>& asset)
		{ // iterate through all lights in the scene and draw their settings
			// dynamically cast the asset to a Light object
			auto light = dynamic_cast<Light*>(asset.get());

			drawLightControls(light); // draw controls for each light in the scene

			ImGui::Separator(); // add a separator between lights
		});

		std::for_each(assetManager->GetAssets("MODEL").begin(),
					  assetManager->GetAssets("MODEL").end(),
					  [&](const std::shared_ptr<Asset>& asset)
		{ // iterate through all models in the scene and draw their settings
			// dynamically cast the asset to a Model object
			auto model = dynamic_cast<Model*>(asset.get());

			drawModelControls(model); // draw controls for each model in the scene

			ImGui::Separator(); // add a separator between models
		});

		ImGui::End(); // end the Scene Settings window
	}
}
void GUI::drawAddObjectWindow()
{
	// set initial size and position for the add object window
	ImGui::SetNextWindowSize(m_addObjectWindowSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(m_addObjectWindowPosition, ImGuiCond_Appearing);
	// set the add object window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that contains buttons to add new objects to the scene
		ImGui::Begin("Add Objects");

		// button to add a new directional light to the scene
		if (ImGui::Button("Add Directional Light", ImVec2(ITEM_WIDTH, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Add Directional Light");
			// set random initial values
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f),
																 MIN_DISTANCE_TO_ORIGIN,
																 MAX_DISTANCE_TO_ORIGIN);
			m_newDirection = m_randomizer->GenerateRandomDirection();
		}
		drawAddDirectionalLightPopup(); // draw the popup for adding a new directional light

		// button to add a new spotlight to the scene
		if (ImGui::Button("Add Spotlight", ImVec2(ITEM_WIDTH, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Add Spotlight");
			// set random initial values
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f),
																 MIN_DISTANCE_TO_ORIGIN,
																 MAX_DISTANCE_TO_ORIGIN);
			m_newDirection = m_randomizer->GenerateRandomDirection();
		}
		drawAddSpotlightPopup(); // draw the popup for adding a new spotlight

		// button to add a new point light to the scene
		if (ImGui::Button("Add Point Light", ImVec2(ITEM_WIDTH, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Add Point Light");
			// set random initial values
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f),
																 MIN_DISTANCE_TO_ORIGIN,
																 MAX_DISTANCE_TO_ORIGIN);
		}
		drawAddPointLightPopup(); // draw the popup for adding a new point light

		// buttom to add a new cube shape to the scene
		if (ImGui::Button("Add Cube Shape", ImVec2(ITEM_WIDTH, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Add Cube Shape");
			// set random initial values
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f),
																 MIN_DISTANCE_TO_ORIGIN,
																 MAX_DISTANCE_TO_ORIGIN);
		}
		drawAddCubeShapePopup(); // draw the popup for adding a new cube shape

		// button to import a new model from a file
		if (ImGui::Button("Import 3D Model", ImVec2(ITEM_WIDTH, 0.0f)))
		{ // if the button is clicked
			// file dialog configuration
			IGFD::FileDialogConfig fileDialogConfig;
			fileDialogConfig.path = "."; // initial directory to open the file dialog
			fileDialogConfig.countSelectionMax = 1; // for now, allow only one file to be selected
			fileDialogConfig.flags = ImGuiFileDialogFlags_Modal; // no special flags for the file dialog

			// open a file dialog to select a model file
			ImGuiFileDialog::Instance()->OpenDialog(
				"ChooseFileDlgKey", // unique key for the file dialog
				"Choose Model File", // title of the file dialog
				".obj, .fbx, .dae, .gltf, .glb, .stl, .ply, .3ds, .max", // supported file extensions
				fileDialogConfig // file dialog configuration
			);
		}
		drawImportModelPopup(); // draw the popup for importing a new model

		ImGui::End(); // end the Add Objects window
	}
}

void GUI::drawCamerasInformation() const
{
	// get the asset manager and scene manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();
	auto& sceneManager = Core::GetInstance()->GetSceneManager();

	// get the number of cameras in the scene
	unsigned int nCameras = assetManager->GetNCameras();
	// display the number of cameras in the scene
	ImGui::Text("\n");
	ImGui::Text("Number of Cameras: %d", nCameras);

	// get the camera object
	auto& camera = sceneManager->GetCamera();
	// display the name of the camera and its position
	ImGui::Text("\n");
	ImGui::Text("CAMERAS");
	ImGui::Text("\n");
	ImGui::Text("%s", camera->GetName().c_str());
	ImGui::Text("Camera Position: (%.3f, %.3f, %.3f)",
				camera->GetPosition().x,
				camera->GetPosition().y,
				camera->GetPosition().z);
	ImGui::Text("\n");
}
void GUI::drawLightsInformation() const
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	// get the number of lights and of each type of light in the scene
	unsigned int nLights = assetManager->GetNLights();
	unsigned int nPointLights = assetManager->GetNPointLights();
	unsigned int nSpotlights = assetManager->GetNSpotlights();
	unsigned int nDirectionalLights = assetManager->GetNDirectionalLights();
	// display the number of lights and each type of light in the scene
	ImGui::Text("\n");
	ImGui::Text("Number of Lights: %d", nLights);
	ImGui::Text("Number of Point Lights: %d", nPointLights);
	ImGui::Text("Number of Spotlights: %d", nSpotlights);
	ImGui::Text("Number of Directional Lights: %d", nDirectionalLights);

	// display the attributes of each light in the scene
	ImGui::Text("\n");
	ImGui::Text("LIGHTS:");
	ImGui::Text("\n");
	std::for_each(assetManager->GetAssets("LIGHT").begin(),
				  assetManager->GetAssets("LIGHT").end(),
				  [&](const std::shared_ptr<Asset>& asset)
	{ // iterate over all lights in the asset manager and display their attributes
		// dynamically cast the asset to a Light object
		auto light = dynamic_cast<Light*>(asset.get());

		switch (light->GetLightType()) // switch based on the type of the light
		{
			case LightType::DIRECTIONAL_LIGHT: // if the Light is a DirectionalLight
			{
				// get the DirectionalLight object
				auto directionalLight = dynamic_cast<DirectionalLight*>(light);

				// use PushID to create a unique ID for each light 
				ImGui::PushID(directionalLight->GetName().c_str());

				// display the attributes of the directional light (position is just for visualization)
				ImGui::Text("%s", directionalLight->GetName().c_str());
				ImGui::Text("\tColor: (%.3f, %.3f, %.3f)",
							directionalLight->GetDiffuse().x,
							directionalLight->GetDiffuse().y,
							directionalLight->GetDiffuse().z);
				ImGui::Text("\tPosition: (%.3f, %.3f, %.3f)",
							directionalLight->GetPosition().x,
							directionalLight->GetPosition().y,
							directionalLight->GetPosition().z);
				ImGui::Text("\tDirection: (%.3f, %.3f, %.3f)",
							directionalLight->GetDirection().x,
							directionalLight->GetDirection().y,
							directionalLight->GetDirection().z);

				ImGui::PopID(); // use PopID to end the unique ID scope
			}
			break;
			case LightType::POINT_LIGHT: // if the Light is a PointLight
			{
				// dynamically cast the asset to a PointLight object
				auto pointLight = dynamic_cast<PointLight*>(light);

				// use PushID to create a unique ID for each light 
				ImGui::PushID(pointLight->GetName().c_str());

				// display the attributes of the point light
				ImGui::Text("%s", pointLight->GetName().c_str());
				ImGui::Text("\tColor: (%.3f, %.3f, %.3f)",
							pointLight->GetDiffuse().x,
							pointLight->GetDiffuse().y,
							pointLight->GetDiffuse().z);
				ImGui::Text("\tPosition: (%.3f, %.3f, %.3f)",
							pointLight->GetPosition().x,
							pointLight->GetPosition().y,
							pointLight->GetPosition().z);

				ImGui::PopID(); // use PopID to end the unique ID scope
			}
			break;
			case LightType::SPOTLIGHT: // if the Light is a Spotlight
			{
				// dynamically cast the asset to a Spotlight object
				auto spotlight = dynamic_cast<Spotlight*>(light);

				// use PushID to create a unique ID for each spotlight 
				ImGui::PushID(spotlight->GetName().c_str());

				// display the attributes of the spotlight
				ImGui::Text("%s", spotlight->GetName().c_str());
				ImGui::Text("\tColor: (%.3f, %.3f, %.3f)",
							spotlight->GetDiffuse().x,
							spotlight->GetDiffuse().y,
							spotlight->GetDiffuse().z);
				ImGui::Text("\tPosition: (%.3f, %.3f, %.3f)",
							spotlight->GetPosition().x,
							spotlight->GetPosition().y,
							spotlight->GetPosition().z);
				ImGui::Text("\tDirection: (%.3f, %.3f, %.3f)",
							spotlight->GetDirection().x,
							spotlight->GetDirection().y,
							spotlight->GetDirection().z);
				ImGui::Text("\tInner cut-off: %.3f", glm::degrees(glm::acos(spotlight->GetInnerCutOff())));
				ImGui::Text("\tOuter cut-off: %.3f", glm::degrees(glm::acos(spotlight->GetOuterCutOff())));

				ImGui::PopID(); // use PopID to end the unique ID scope
			}
			break;
			case LightType::UNDEFINED: // if the Light is of an undefined type
				std::cerr << "[ERROR::GUI::drawLightsInformation] UNDEFINED light type for light: "
					<< light->GetName() << std::endl;
				return;
			default: // if the Light is of an unknown type
				std::cerr << "[ERROR::GUI::drawLightsInformation] Unknown light type for light: "
					<< light->GetName() << std::endl;
				return;
		}
	});
	ImGui::Text("\n");
}
void GUI::drawModelsInformation() const
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	// get the number of models and of each type of model in the scene
	unsigned int nModels = assetManager->GetNModels();
	unsigned int nShapes = assetManager->GetNShapes();
	unsigned int nAssimpModels = assetManager->GetNAssimpModels();
	// display the number of models and each type of model in the scene
	ImGui::Text("\n");
	ImGui::Text("Number of Models: %d", nModels);
	ImGui::Text("\tNumber of Shapes: %d", nShapes);
	ImGui::Text("\tNumber of Assimp models: %d", nAssimpModels);

	// display the attributes of each model in the scene
	ImGui::Text("\n");
	ImGui::Text("MODELS:");
	ImGui::Text("\n");
	std::for_each(assetManager->GetAssets("MODEL").begin(),
				  assetManager->GetAssets("MODEL").end(),
				  [&](const std::shared_ptr<Asset>& asset)
	{ // iterate over all models in the asset manager and display their attributes
		// dynamically cast the asset to a Model object
		auto model = dynamic_cast<Model*>(asset.get());

		ImGui::PushID(model->GetName().c_str()); // use PushID to create a unique ID for each model

		// display the attributes of the model
		ImGui::Text("Name: %s", model->GetName().c_str());
		ImGui::Text("\tColor: (%.3f, %.3f, %.3f)",
					model->GetAlbedo().x,
					model->GetAlbedo().y,
					model->GetAlbedo().z);
		ImGui::Text("\tPosition: (%.3f, %.3f, %.3f)",
					model->GetPosition().x,
					model->GetPosition().y,
					model->GetPosition().z);
		ImGui::Text("\tRotation: (%.3f, %.3f, %.3f)",
					model->GetRotationInEulerAngles().x,
					model->GetRotationInEulerAngles().y,
					model->GetRotationInEulerAngles().z);
		ImGui::Text("\tScale: (%.3f, %.3f, %.3f)",
					model->GetScale().x,
					model->GetScale().y,
					model->GetScale().z);
		ImGui::Text("\tForward: (%.3f, %.3f, %.3f)",
					model->GetForward().x,
					model->GetForward().y,
					model->GetForward().z);

		ImGui::PopID(); // use PopID to end the unique ID scope
	});
	ImGui::Text("\n");
}

void GUI::drawLightControls(Light* light)
{
	switch (light->GetLightType()) // switch based on the type of the light
	{
		case LightType::DIRECTIONAL_LIGHT: // if the Light is a DirectionalLight
		{
			// dynamically cast the light to a DirectionalLight object
			auto directionalLight = dynamic_cast<DirectionalLight*>(light);
			// draw controls for the directional light
			drawDirectionalLightControls(directionalLight);
		}
		break;
		case LightType::POINT_LIGHT: // if the Light is a PointLight
		{
			// dynamically cast the light to a PointLight object
			auto pointLight = dynamic_cast<PointLight*>(light);
			// draw controls for the point light
			drawPointLightControls(dynamic_cast<PointLight*>(light));
		}
		break;
		case LightType::SPOTLIGHT: // if the Light is a Spotlight
		{
			// dynamically cast the light to a Spotlight object
			auto spotlight = dynamic_cast<Spotlight*>(light);
			// draw controls for the spotlight
			drawSpotlightControls(dynamic_cast<Spotlight*>(light));
		}
		break;
		case LightType::UNDEFINED: // if the Light is of an undefined type
			std::cerr << "[ERROR::GUI::drawLightControls] UNDEFINED light type for light: "
				<< light->GetName() << std::endl;
			return;
		default: // if the Light is of an unknown type
			std::cerr << "[ERROR::GUI::drawLightControls] Unknown light type for light: "
				<< light->GetName() << std::endl;
			return;
	}
}
void GUI::drawDirectionalLightControls(DirectionalLight* directionalLight)
{
	// use PushID to create a unique ID for each directional light 
	ImGui::PushID(directionalLight->GetName().c_str());

	// display the name of the directional light
	ImGui::Text("%s", directionalLight->GetName().c_str());

	// get the color of the directional light
	glm::vec3 color = directionalLight->GetDiffuse();
	// create a color picker for the directional light's color
	if (drawColorControl("Albedo", color))
	{ // if the color control is used
		directionalLight->SetDiffuse(color); // set the new color of the directional light
		directionalLight->SyncGizmoColorFromLight(); // set the new color of the gizmo
	}

	// get the position of the directional light (it only serves visualization purposes)
	glm::vec3 pos = directionalLight->GetPosition();
	// create a control for the x, y, and z components of the directional light's position
	if (drawVec3Control("Position", pos, false, // is not the scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE))
	{ // if the control is used
		directionalLight->SetPosition(pos); // set the new position of the directional light
		directionalLight->SyncGizmoPositionFromLight(); // set the new position of the gizmo
	}

	// retrieve the gizmo of the directional light and its rotation in Euler angles
	auto gizmo = directionalLight->GetGizmo();
	glm::vec3 rotDegrees = gizmo->GetRotationInEulerAngles();
	// create a control for the x, y, and z components of the directional light's rotation
	if (drawVec3Control("Rotation", rotDegrees, false, // is not the scale control
						MIN_ROTATION_VALUE, MAX_ROTATION_VALUE))
	{ // if the control is used
					// set the new rotation of the directional light's gizmo
		gizmo->SetRotationInEulerAngles(rotDegrees);
		// update the light's direction based on the gizmo's new forward vector,
		// without causing the gizmo to be re-oriented by SetForward() again
		directionalLight->SetDirectionOnly(gizmo->GetForward());
	}

	ImGui::PopID(); // use PopID to end the unique ID scope
}
void GUI::drawPointLightControls(PointLight* pointLight)
{
	// use PushID to create a unique ID for each point light 
	ImGui::PushID(pointLight->GetName().c_str());

	// display the name of the point light
	ImGui::Text("%s", pointLight->GetName().c_str());

	// get the color of the point light
	glm::vec3 color = pointLight->GetDiffuse();
	// create a color picker for the point light's color
	if (drawColorControl("Albedo", color))
	{ // if the color control is used
		pointLight->SetDiffuse(color); // set the new color of the point light
		pointLight->SyncGizmoColorFromLight(); // set the new color of the gizmo
	}

	// get the position of the point light
	glm::vec3 pos = pointLight->GetPosition();
	// create a control for the x, y, and z components of the point light's position
	if (drawVec3Control("Position", pos, false, // is not the scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE))
	{ // if the control is used
		pointLight->SetPosition(pos); // set the new position of the point light
		pointLight->SyncGizmoPositionFromLight(); // set the new position of the gizmo
	}

	ImGui::PopID(); // use PopID to end the unique ID scope
}
void GUI::drawSpotlightControls(Spotlight* spotlight)
{
	// use PushID to create a unique ID for each spotlight
	ImGui::PushID(spotlight->GetName().c_str());

	// display the name of the spotlight
	ImGui::Text("%s", spotlight->GetName().c_str());

	// get the color of the spotlight
	glm::vec3 color = spotlight->GetDiffuse();
	// create a color picker for the spotlight's color
	if (drawColorControl("Albedo", color))
	{ // if the color control is used
		spotlight->SetDiffuse(color); // set the new color of the spotlight
		spotlight->SyncGizmoColorFromLight(); // set the new color of the gizmo
	}

	// get the position of the spotlight
	glm::vec3 pos = spotlight->GetPosition();
	// create a control for the x, y, and z components of the spotlight's position
	if (drawVec3Control("Position", pos, false, // is not the scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE))
	{ // if the control is used
		spotlight->SetPosition(pos); // set the new position of the spotlight
		spotlight->SyncGizmoPositionFromLight(); // set the new position of the gizmo
	}

	// retrieve the gizmo of the spotlight and its rotation in Euler angles
	auto gizmo = spotlight->GetGizmo();
	glm::vec3 rotDegrees = gizmo->GetRotationInEulerAngles();
	// create a control for the x, y, and z components of the spotlight's rotation
	if (drawVec3Control("Rotation", rotDegrees, false, // is not the scale control
						MIN_ROTATION_VALUE, MAX_ROTATION_VALUE,
						INPUT_FIELD_WIDTH, 0.75f))
	{ // if the control are used
					// set the new rotation of the spotlight's gizmo
		gizmo->SetRotationInEulerAngles(rotDegrees);
		// update the light's direction based on the gizmo's new forward vector,
		// without causing the gizmo to be re-oriented by SetForward() again
		spotlight->SetDirectionOnly(gizmo->GetForward());
	}

	// get the inner and outer cut-off angles of the spotlight and 
	// convert them from radians (cosine) to degrees for the controls
	float innerCutOff = glm::degrees(glm::acos(spotlight->GetInnerCutOff()));
	float outerCutOff = glm::degrees(glm::acos(spotlight->GetOuterCutOff()));
	// create controls for the inner and outer cut-off angles of the spotlight
	ImGui::Text("Cut-off Angles (in degrees):");
	if (drawFloatControl("Inner", innerCutOff,
						 MIN_CUTOFF_VALUE, outerCutOff, // innerCutOff <= outerCutOff
						 INPUT_FIELD_WIDTH, 0.1f, INNER_CUTOFF_VALUE))
	{ // if the control is used
					// convert back to radians and cosine and
					// set the new inner cut-off angle for the spotlight
		spotlight->SetInnerCutOff(glm::cos(glm::radians(innerCutOff)));
	}
	if (drawFloatControl("Outer", outerCutOff,
						 innerCutOff, MAX_CUTOFF_VALUE, // outerCutOff >= innerCutOff
						 INPUT_FIELD_WIDTH, 0.1f, OUTER_CUTOFF_VALUE))
	{ // if the control is used
					// convert back to radians and cosine and
					// set the new outer cut-off angle for the spotlight
		spotlight->SetOuterCutOff(glm::cos(glm::radians(outerCutOff)));
	}

	ImGui::PopID(); // use PopID to end the unique ID scope
}
void GUI::drawModelControls(Model* model)
{
	// use PushID to create a unique ID for each model
	ImGui::PushID(model->GetName().c_str());

	// display the name of the model
	ImGui::Text("%s", model->GetName().c_str());

	// only if the model is a shape, display the color picker
	if (model->GetModelType() == ModelType::SHAPE)
	{
		// get the color of the model
		glm::vec3 color = model->GetAlbedo();
		// create a color picker for the model's color
		if (drawColorControl("Albedo", color))
		{ // if the color control is used
			model->SetAlbedo(color); // set the new color of the model
		}
	}

	// get the position of the model
	glm::vec3 pos = model->GetPosition();
	// create a control for the x, y, and z components of the model's position
	if (drawVec3Control("Position", pos, false, // is not the scale control
						MIN_POSITION_VALUE, MAX_POSITION_VALUE))
	{ // if the control is used
		model->SetPosition(pos); // set the new position of the model
	}

	// get the rotation in Euler angles
	glm::vec3 rotDegrees = model->GetRotationInEulerAngles();
	// create a control for the x, y, and z components of the model's rotation
	if (drawVec3Control("Rotation", rotDegrees, false, // is not the scale control
						MIN_ROTATION_VALUE, MAX_ROTATION_VALUE,
						INPUT_FIELD_WIDTH, 0.5f))
	{ // if the control is used
		model->SetRotationInEulerAngles(rotDegrees); // set the new rotation of the model
	}

	// get the scale of the model
	glm::vec3 scale = model->GetScale();
	// create a control for the x, y, and z components of the model's scale
	if (drawVec3Control("Scale", scale, true, // is the scale control
						MIN_SCALE_VALUE, MAX_SCALE_VALUE,
						INPUT_FIELD_WIDTH, 0.0005f, 1.0f))
	{ // if the control is used
		model->SetScale(scale); // set the new scale of the model
	}

	ImGui::PopID(); // use PopID to end the unique ID scope
}

void GUI::drawAddDirectionalLightPopup()
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	if (ImGui::BeginPopupModal("Add Directional Light", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::Text("Set initial properties for the new Directional Light.\n");
		ImGui::Separator();

		// display controls to set the color, position, and direction of the new directional light
		drawColorControl("Color", m_newAlbedo);
		drawVec3Control("Position", m_newPosition, false,
						MIN_POSITION_VALUE, MAX_POSITION_VALUE);
		drawVec3Control("Direction", m_newDirection, false, -1.0f, 1.0f);

		ImGui::Separator();

		// display a button to randomize the properties of the new directional light
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new directional light's properties
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																 MIN_DISTANCE_TO_ORIGIN,
																 MAX_DISTANCE_TO_ORIGIN);
			m_newDirection = m_randomizer->GenerateRandomDirection();
		}

		// display a button to create the new directional light
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Create button is clicked
			// get the number of models and directional lights in the scene
			std::string nModels = std::to_string(assetManager->GetNModels());
			std::string nDirectionalLights =
				std::to_string(assetManager->GetNDirectionalLights());

			// create a new directional light with the specified properties
			auto newDirectionalLight = std::make_shared<DirectionalLight>(
				"Directional Light " + nDirectionalLights,
				glm::vec3{ 0.1f }, m_newAlbedo, glm::vec3{ 1.0f },
				m_newPosition, m_newDirection
			);

			// get the gizmo of the new directional light before adding the light to the engine
			auto newDirectionalLightGizmo = newDirectionalLight->GetGizmo();
			// add the new directional light to the engine
			assetManager->AddAsset(std::move(newDirectionalLight));
			// set the name of the gizmo to include the model number
			newDirectionalLightGizmo->SetName(newDirectionalLightGizmo->GetName() + " (Model " + nModels + ")");
			// add the gizmo of the new directional light to the engine
			assetManager->AddAsset(std::move(newDirectionalLightGizmo));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}

		ImGui::EndPopup();
	}
}
void GUI::drawAddPointLightPopup()
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	if (ImGui::BeginPopupModal("Add Point Light", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::Text("Set initial properties for the new Point Light\n");
		ImGui::Separator();

		// display controls to set the color and position of the new point light
		drawColorControl("Color", m_newAlbedo);
		drawVec3Control("Position", m_newPosition, false,
						MIN_POSITION_VALUE, MAX_POSITION_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new point light
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new point light's properties
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																 MIN_DISTANCE_TO_ORIGIN,
																 MAX_DISTANCE_TO_ORIGIN);
		}

		ImGui::SameLine();
		// display a button to create the new point light
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Create button is clicked
			// get the number of models and point lights in the scene
			std::string nModels = std::to_string(assetManager->GetNModels());
			std::string nPointLights = std::to_string(assetManager->GetNPointLights());

			// create a new point light with the specified properties
			auto newPointLight = std::make_shared<PointLight>(
				"Point Light " + nPointLights,
				glm::vec3{ 0.1f }, m_newAlbedo, glm::vec3{ 1.0f },
				m_newPosition
			);

			// get the gizmo of the new point light before adding the light to the engine
			auto newPointLightGizmo = newPointLight->GetGizmo();
			// add the new point light to the engine
			assetManager->AddAsset(std::move(newPointLight));
			// set the name of the gizmo to include the model number
			newPointLightGizmo->SetName(newPointLightGizmo->GetName() + " (Model " + nModels + ")");
			// add the gizmo of the new point light to the engine
			assetManager->AddAsset(std::move(newPointLightGizmo));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}
		ImGui::EndPopup();
	}
}
void GUI::drawAddSpotlightPopup()
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	if (ImGui::BeginPopupModal("Add Spotlight", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::Text("Set initial properties for the new Spotlight\n");
		ImGui::Separator();

		// display controls to set the color, position, direction, and cut-off angles of the new spotlight
		drawColorControl("Color", m_newAlbedo);
		drawVec3Control("Position", m_newPosition, false,
						MIN_POSITION_VALUE, MAX_POSITION_VALUE);
		drawVec3Control("Direction", m_newDirection, false, -1.0f, 1.0f);
		ImGui::Text("Cut-Off Angles (in degrees):");
		// initialize the inner and outer cut-off angles with default values before drawing the controls
		drawFloatControl("Inner", m_newInnerCutOff,
						 MIN_CUTOFF_VALUE, MAX_CUTOFF_VALUE, // innerCutOff <= outerCutOff
						 INPUT_FIELD_WIDTH, 0.1f, INNER_CUTOFF_VALUE);
		drawFloatControl("Outer", m_newOuterCutOff,
						 m_newInnerCutOff, MAX_CUTOFF_VALUE, // outerCutOff >= innerCutOff
						 INPUT_FIELD_WIDTH, 0.1f, OUTER_CUTOFF_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new spotlight
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new spotlight's properties
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																 MIN_DISTANCE_TO_ORIGIN,
																 MAX_DISTANCE_TO_ORIGIN);
			m_newDirection = m_randomizer->GenerateRandomDirection();
			m_newInnerCutOff = m_randomizer->GenerateRandomFloat(
				MIN_CUTOFF_VALUE,
				MAX_CUTOFF_VALUE);
			m_newOuterCutOff = m_randomizer->GenerateRandomFloat(
				m_newInnerCutOff, // ensure outer cut-off is greater than inner cut-off
				MAX_CUTOFF_VALUE);
		}

		// display a button to create the new spotlight
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Create button is clicked
			// get the number of models and spotlights in the scene
			std::string nModels = std::to_string(assetManager->GetNModels());
			std::string nSpotlights = std::to_string(assetManager->GetNSpotlights());

			// create a new spotlight with the specified properties
			auto newSpotlight = std::make_shared<Spotlight>(
				"Spotlight " + nSpotlights,
				glm::vec3{ 0.1f }, m_newAlbedo, glm::vec3{ 1.0f },
				m_newPosition, m_newDirection
			);

			// get the gizmo of the new spotlight before adding the light to the engine
			auto newSpotlightGizmo = newSpotlight->GetGizmo();
			// add the new spotlight to the engine
			assetManager->AddAsset(std::move(newSpotlight));
			// set the name of the gizmo to include the model number
			newSpotlightGizmo->SetName(newSpotlightGizmo->GetName() + " (Model " + nModels + ")");
			// add the gizmo of the new spotlight to the engine
			assetManager->AddAsset(std::move(newSpotlightGizmo));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}

		ImGui::EndPopup();
	}
}
void GUI::drawAddCubeShapePopup()
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	if (ImGui::BeginPopupModal("Add Cube Shape", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::Text("Set initial properties for the new Cube Shape\n");
		ImGui::Separator();

		// display controls to set the color and position of the new cube shape
		drawColorControl("Color", m_newAlbedo);
		drawVec3Control("Position", m_newPosition, false,
						MIN_POSITION_VALUE, MAX_POSITION_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new cube shape
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new cube shape's properties
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(glm::vec3(0.0f), // origin
																 MIN_DISTANCE_TO_ORIGIN,
																 MAX_DISTANCE_TO_ORIGIN);
		}

		// display a button to create the new cube shape
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Create button is clicked
			// get the number of models in the scene
			std::string nModels = std::to_string(assetManager->GetNModels());

			// create a cube shape with the specified properties
			auto newCubeShape = std::make_shared<Shape>(
				"Cube (Model " + nModels + ")",
				cubeVerticesVec, cubeIndicesVec,
				m_newAlbedo,
				m_newPosition
			);

			// add the new cube shape to the engine
			assetManager->AddAsset(std::move(newCubeShape));

			ImGui::CloseCurrentPopup(); // close the popup
		}

		// display a button to cancel the operation and close the popup
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Cancel button is clicked
			ImGui::CloseCurrentPopup(); // close the popup
		}

		ImGui::EndPopup();
	}
}
void GUI::drawImportModelPopup()
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for display size

	// set the size and position of the next window to display the file dialog adequately,
	// centered on the display and with a predefined size
	ImGui::SetNextWindowSize(ImVec2(FILE_DIALOG_POPUP_WIDTH, FILE_DIALOG_POPUP_HEIGHT),
							 ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f - FILE_DIALOG_POPUP_WIDTH * 0.5f,
								   io.DisplaySize.y * 0.5f - FILE_DIALOG_POPUP_HEIGHT * 0.5f),
							ImGuiCond_Appearing);

	// check if the file dialog is displayed and if the user selected a file
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{ // if the file dialog is displayed
		if (ImGuiFileDialog::Instance()->IsOk())
		{ // if the user clicked the OK button (i.e. selected a file)
			// get the selected file path and name
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

			// normalize slashes to forward slashes for cross-platform texture loading
			std::replace(filePathName.begin(), filePathName.end(), '\\', '/');

			// get the current number of models in the scene
			std::string nModels = std::to_string(assetManager->GetNModels());

			// create a new model from the selected file
			auto newAssimpModel = std::make_shared<AssimpModel>(
				fileName + " (Model " + nModels + ")",
				filePathName
			);

			// add the new model to the engine
			assetManager->AddAsset(std::move(newAssimpModel));
		}

		ImGuiFileDialog::Instance()->Close(); // close the file dialog
	}
}

bool GUI::drawColorControl(const std::string& label, glm::vec3& color,
						   float colorControlWidth)
{
	bool value_changed{ false }; // flag to indicate if the color has changed

	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for font and style settings
	auto boldFont = io.Fonts->Fonts[0]; // get the bold font from the ImGui IO object

	ImGui::PushID(label.c_str()); // create a unique ID for the label to avoid conflicts with other controls

	ImGui::Text("%s", label.c_str()); // display the label for the control

	ImGui::SetNextItemWidth(colorControlWidth); // set the width of the color picker
	if (ImGui::ColorEdit3("##color", (float*)&color))
	{ // if the color picker is used
		value_changed = true; // set the value_changed flag to true
	}

	ImGui::PopID(); // end the unique ID scope for the label

	return value_changed; // return whether the color has changed
}
bool GUI::drawVec3Control(const std::string& label, glm::vec3& values, bool scaleControls,
						  float minInputFieldValue, float maxInputFieldValue,
						  float inputFieldWidth, float speed,
						  float resetValue, float resetButtonWidth)
{
	bool value_changed{ false }; // flag to indicate if any value has changed

	ImGui::PushID(label.c_str()); // create a unique ID for the label to avoid conflicts with other controls

	ImGui::Text("%s", label.c_str()); // display the label for the control

	// if the controls are for scaling, display a checkbox to enable proportional scaling
	if (scaleControls)
		ImGui::Checkbox("Proportional", &s_proportionalScaling);

	// store previous values so that we can check if any value has changed
	glm::vec3 prevValues = values;

	// x component
	ImGui::PushID("x");
	ImGui::Text("  X  "); // display the label for the x component
	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputFieldWidth); // set the width of the input field for the x component
	if (ImGui::InputFloat("##x", &values.x, 0.0f, 0.0f, "%.3f"))
	{ // if the input field is used
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##upX", ImGuiDir_Up))
	{ // if the up arrow button is pressed
		values.x += speed; // increase the x component by speed
		value_changed = true; // set the value_changed flag to true
	}
	if (ImGui::IsItemActive())
	{ // if the up arrow button is active (held down)
		values.x += speed; // increase the x component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##downX", ImGuiDir_Down))
	{ // if the down arrow button is pressed
		values.x -= speed; // decrease the x component by speed
		value_changed = true; // set the value_changed flag to true
	}
	if (ImGui::IsItemActive()) // if the down arrow button is active (held down)
	{
		values.x -= speed; // decrease the x component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(resetButtonWidth, 20.0f)))
	{ // if the Reset button is pressed
		values.x = resetValue; // reset the x component to the reset value
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopID(); // end the unique ID scope for the x component

	// y component
	ImGui::PushID("y");
	ImGui::Text("  Y  "); // display the label for the y component
	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputFieldWidth); // set the width of the input field for the y component
	if (ImGui::InputFloat("##y", &values.y, 0.0f, 0.0f, "%.3f"))
	{ // if the input field is used
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##upY", ImGuiDir_Up))
	{ // if the up arrow button is pressed
		values.y += speed; // increase the y component by speed
		value_changed = true; // set the value_changed flag to true
	}
	if (ImGui::IsItemActive())
	{ // if the up arrow button is active (held down)
		values.y += speed; // increase the y component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##downY", ImGuiDir_Down))
	{ // if the down arrow button is pressed
		values.y -= speed; // decrease the y component by speed
		value_changed = true; // set the value_changed flag to true
	}
	if (ImGui::IsItemActive())
	{ // if the down arrow button is active (held down)
		values.y -= speed; // decrease the y component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(resetButtonWidth, 20.0f)))
	{ // if the Reset button is pressed
		values.y = resetValue; // reset the y component to the reset value
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopID(); // end the unique ID scope for the y component

	// z component
	ImGui::PushID("z");
	ImGui::Text("  Z  "); // display the label for the z component
	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputFieldWidth); // set the width of the input field for the z component
	if (ImGui::InputFloat("##z", &values.z, 0.0f, 0.0f, "%.3f"))
	{ // if the input field is used
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##upZ", ImGuiDir_Up))
	{ // if the up arrow button is pressed
		values.z += speed; // increase the z component by speed
		value_changed = true; // set the value_changed flag to true
	}
	if (ImGui::IsItemActive()) // if the up arrow button is active (held down)
	{
		values.z += speed; // increase the z component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##downZ", ImGuiDir_Down))
	{ // if the down arrow button is pressed
		values.z -= speed; // decrease the z component by speed
		value_changed = true; // set the value_changed flag to true
	}
	if (ImGui::IsItemActive()) // if the down arrow button is active (held down)
	{
		values.z -= speed; // decrease the z component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(resetButtonWidth, 20.0f)))
	{ // if the Reset button is pressed
		values.z = resetValue; // reset the z component to the reset value
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopID(); // end the unique ID scope for the z component

	ImGui::PopID(); // end the unique ID scope for the label

	// clamp values to the specified range
	values.x = std::clamp(values.x, minInputFieldValue, maxInputFieldValue);
	values.y = std::clamp(values.y, minInputFieldValue, maxInputFieldValue);
	values.z = std::clamp(values.z, minInputFieldValue, maxInputFieldValue);

	// if proportional scaling is enabled, adjust the other components accordingly
	if (scaleControls && s_proportionalScaling)
	{
		if (values.x != prevValues.x)
		{
			values.y = values.z = values.x;
			value_changed = true;
		}
		else if (values.y != prevValues.y)
		{
			values.x = values.z = values.y;
			value_changed = true;
		}
		else if (values.z != prevValues.z)
		{
			values.x = values.y = values.z;
			value_changed = true;
		}
	}

	// update the value_changed flag if any of the components have changed
	if (values.x != prevValues.x || values.y != prevValues.y || values.z != prevValues.z)
		value_changed = true;

	return value_changed;
}
bool GUI::drawFloatControl(const std::string& label, float& value,
						   float minInputFieldValue, float maxInputFieldValue,
						   float inputFieldWidth, float speed,
						   float resetValue, float resetButtonWidth)
{
	bool value_changed{ false }; // flag to indicate if the value has changed

	ImGui::PushID(label.c_str()); // create a unique ID for the label to avoid conflicts with other controls

	ImGui::Text("%s", label.c_str()); // display the label for the control

	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputFieldWidth); // set the width of the input field
	if (ImGui::InputFloat("##val", &value, 0.0f, 0.0f, "%.2f"))
	{ // if the input field is used
		value_changed = true;
	} // set the value_changed flag to true

	ImGui::SameLine();
	if (ImGui::ArrowButton("##up", ImGuiDir_Up))
	{ // if the up arrow button is pressed
		value += speed; // increase the value by speed
		value_changed = true; // set the value_changed flag to true
	}
	if (ImGui::IsItemActive())
	{ // if the up arrow button is active (held down)
		value += speed; // increase the value by speed
		value_changed = true; // set the value_changed flag to true
	}

	ImGui::SameLine();
	if (ImGui::ArrowButton("##down", ImGuiDir_Down))
	{ // if the down arrow button is pressed
		value -= speed; // decrease the value by speed
		value_changed = true; // set the value_changed flag to true
	}
	if (ImGui::IsItemActive())
	{ // if the down arrow button is active (held down)
		value -= speed; // decrease the value by speed
		value_changed = true; // set the value_changed flag to true
	}

	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(resetButtonWidth, 20.0f)))
	{ // if the Reset button is pressed
		value = resetValue; // reset the value to the reset value
		value_changed = true; // set the value_changed flag to true
	}

	// clamp the value to the specified range
	value = std::clamp(value, minInputFieldValue, maxInputFieldValue);

	ImGui::PopID(); // end the unique ID scope for the label

	return value_changed;
}