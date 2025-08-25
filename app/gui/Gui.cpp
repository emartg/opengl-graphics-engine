/*
* GUI.cpp
* This file implements the GUI class, which is used to create a graphical user interface
* using the ImGui library.
* The engine will use this class to create a windows to display information about the scene
* and allow the user to interact with it, e.g. change certain parameters or create new objects.
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
	: m_randomizer{ std::make_unique<Random>() }, // create a random number generator
	m_newAlbedo{ 0.8f }, // default albedo color for new objects is light gray
	m_newPosition{ 0.0f }, // default position for new objects is the origin
	m_newRotation{ 0.0f }, // default rotation for new objects is no rotation (identity quaternion)
	m_newDirection{ 0.0f, 0.0f, -1.0f }, // default direction for new objects is negative z-axis
	m_newScale{ 1.0f }, // default scale for new objects is 1.0
	m_newInnerCutOff{ 12.5f }, // default inner cutoff angle for new spotlights
	m_newOuterCutOff{ 32.5f }, // default outer cutoff angle for new spotlights
	m_mediumFont{ nullptr }, // medium font for the GUI (default font) is nullptr initially
	m_boldFont{ nullptr } // bold font for the GUI is nullptr initially
{
	initGUILayoutAttributes(); // initialize the display size-independent layout attributes
	// initialize the positions and sizes of the GUI windows to default values
	// (since they require the ImGui context to be created first to access the ImGui IO object)
	m_informationWindowPosition, m_debugWindowPosition, m_creationWindowPosition, m_propertiesWindowPosition
		= ImVec2{ 0.0f, 0.0f };
	m_informationWindowSize, m_debugWindowSize, m_creationWindowSize, m_propertiesWindowSize
		= ImVec2{ 0.0f, 0.0f };
	// initialize the flags for the windows to prevent focus on the first frame
	m_informationWindowJustAppeared = true;
	m_debugWindowJustAppeared = true;
	m_propertiesWindowJustAppeared = true;
	m_creationWindowJustAppeared = true;
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

	// button behavior configuration
	// faster button repeat when held down
	io.KeyRepeatDelay = 0.25f; // delay before repeating starts (in seconds)
	io.KeyRepeatRate = 0.05f; // rate at which the button is repeated (seconds between repeats)

	configureGUIStyle(); // configure the ImGui style (fonts, colors, etc.)

	// setup platform/renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glslVersion);
}

void GUI::BuildGUI()
{
	beginGUIFrame(); // start a new ImGui frame
	configureGUILayout(); // configure the layout of the GUI windows based on the display size
	drawGUIWindows(); // draw the GUI windows (information, settings, and Create Windows)
	handleImGuiInput(); // handle ImGui input (mouse and keyboard)
}

void GUI::RenderGUI()
{
	ImGui::Render(); // render the ImGui draw data
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::ShutdownGUI() const
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(); // destroy the ImGui context
}

// Private methods
// ---------------
void GUI::initGUILayoutAttributes()
{
	// set the private variables for the GUI layout
	m_informationWindowRelativeWidth = 0.3f;
	m_informationWindowRelativeHeight = 0.6f;
	m_debugWindowRelativeWidth = 0.3f;
	m_debugWindowRelativeHeight = 0.4f;
	m_propertiesWindowRelativeWidth = 0.2f;
	m_propertiesWindowRelativeHeight = 0.8f;
	m_creationWindowRelativeWidth = 0.2f;
	m_creationWindowRelativeHeight = 0.2f;

	m_informationWindowXOffset = 0.0f;
	m_informationWindowYOffset = 0.0f;
	m_debugWindowXOffset = 0.0f;
	m_debugWindowYOffset = 1.0f - m_debugWindowRelativeHeight;
	m_propertiesWindowXOffset = 1.0f - m_propertiesWindowRelativeWidth;
	m_propertiesWindowYOffset = 0.0f;
	m_creationWindowXOffset = 1.0f - m_creationWindowRelativeWidth;
	m_creationWindowYOffset = 1.0f - m_creationWindowRelativeHeight;
	m_windowPositionPadding = ImVec2{ 10.f, 10.0f };
	m_windowSizePadding = ImVec2{ 20.f, 20.0f };
}

void GUI::configureGUIStyle()
{
	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for font and style settings
	ImGuiStyle& style = ImGui::GetStyle(); // get the ImGui style object

	// add custom fonts to the ImGui context, setting one as the default font
	io.Fonts->AddFontDefault();
	ImFont* mediumFont = io.Fonts->AddFontFromFileTTF("resources/fonts/RobotoMono-Medium.ttf", 16.0f);
	ImFont* boldFont = io.Fonts->AddFontFromFileTTF("resources/fonts/RobotoMono-Bold.ttf", 16.0f);
	io.FontDefault = mediumFont; // set the medium font as the default font
	// set the member fonts for the GUI class
	m_mediumFont = mediumFont;
	m_boldFont = boldFont;

	// soften the edges of the ImGui windows and frames
	style.WindowRounding = 5.0f;
	style.FrameRounding = 5.0f;
	style.ScrollbarRounding = 5.0f;
	style.GrabRounding = 5.0f;

	// set the padding for the ImGui style
	style.WindowPadding = ImVec2(12.0f, 12.0f); // padding inside windows
	style.FramePadding = ImVec2(4.0f, 2.0f); // padding inside frames

	// set the ImGui color scheme to dark mode by default
	ImGui::StyleColorsDark();
	// change the color for all ImGui elements with an accent color to a purple hue,
	// leaving the rest of the colors unchanged
	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.08f, 0.15f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.33f, 0.28f, 0.40f, 0.60f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	colors[ImGuiCol_FrameBg] = ImVec4(0.26f, 0.19f, 0.38f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.36f, 0.29f, 0.58f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.36f, 0.29f, 0.58f, 0.67f);

	colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.04f, 0.18f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.26f, 0.19f, 0.38f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.00f, 0.14f, 0.51f);

	colors[ImGuiCol_MenuBarBg] = ImVec4(0.18f, 0.10f, 0.22f, 1.00f);

	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.18f, 0.10f, 0.22f, 0.60f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.21f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.31f, 0.51f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.41f, 0.61f, 1.00f);

	colors[ImGuiCol_CheckMark] = ImVec4(0.46f, 0.29f, 0.68f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.44f, 0.32f, 0.78f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.46f, 0.29f, 0.68f, 1.00f);

	colors[ImGuiCol_Button] = ImVec4(0.56f, 0.39f, 0.78f, 0.60f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.66f, 0.49f, 0.88f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.46f, 0.39f, 0.78f, 1.00f);

	colors[ImGuiCol_Header] = ImVec4(0.36f, 0.29f, 0.58f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.36f, 0.29f, 0.58f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.36f, 0.29f, 0.58f, 1.00f);

	colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.30f, 0.20f, 0.55f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.30f, 0.20f, 0.55f, 1.00f);

	colors[ImGuiCol_ResizeGrip] = ImVec4(0.36f, 0.29f, 0.58f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.36f, 0.29f, 0.58f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.36f, 0.29f, 0.58f, 0.95f);

	colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
	colors[ImGuiCol_TabSelected] = ImLerp(
		colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
	colors[ImGuiCol_TabSelectedOverline] = colors[ImGuiCol_HeaderActive];
	colors[ImGuiCol_TabDimmed] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
	colors[ImGuiCol_TabDimmedSelected] = ImLerp(
		colors[ImGuiCol_TabSelected], colors[ImGuiCol_TitleBg], 0.40f);
	colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);

	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.15f, 0.13f, 0.22f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.21f, 0.21f, 0.25f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.13f, 0.13f, 0.15f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);

	colors[ImGuiCol_TextLink] = colors[ImGuiCol_HeaderActive];
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.36f, 0.29f, 0.58f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavCursor] = ImVec4(0.36f, 0.29f, 0.58f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}


void GUI::beginGUIFrame() const
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void GUI::configureGUILayout()
{
	ImGuiIO& io = ImGui::GetIO(); // get ImGui IO object for font and style settings

	// position of the Information Window (top left corner with padding)
	m_informationWindowPosition = ImVec2{
		io.DisplaySize.x * m_informationWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_informationWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the Information Window
	m_informationWindowSize = ImVec2{
		io.DisplaySize.x * m_informationWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_informationWindowRelativeHeight - m_windowSizePadding.y * 0.5f // height
	};
	// position of the Debug Window (bottom left corner with padding)
	m_debugWindowPosition = ImVec2{
		io.DisplaySize.x * m_debugWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_debugWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the Debug Window
	m_debugWindowSize = ImVec2{
		io.DisplaySize.x * m_debugWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_debugWindowRelativeHeight - m_windowSizePadding.y // height
	};
	// position of the Properties Window (top right corner with padding)
	m_propertiesWindowPosition = ImVec2{
		io.DisplaySize.x * m_propertiesWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_propertiesWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the Properties Window
	m_propertiesWindowSize = ImVec2{
		io.DisplaySize.x * m_propertiesWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_propertiesWindowRelativeHeight - m_windowSizePadding.y * 0.5f // height
	};
	// position of the Create Window (bottom right corner with padding)
	m_creationWindowPosition = ImVec2{
		io.DisplaySize.x * m_creationWindowXOffset + m_windowPositionPadding.x, // x position
		io.DisplaySize.y * m_creationWindowYOffset + m_windowPositionPadding.y // y position
	};
	// size (width and height) of the Create Window
	m_creationWindowSize = ImVec2{
		io.DisplaySize.x * m_creationWindowRelativeWidth - m_windowSizePadding.x, // width
		io.DisplaySize.y * m_creationWindowRelativeHeight - m_windowSizePadding.y // height
	};
}

void GUI::drawGUIWindows()
{
	drawInformationWindow();
	drawDebugWindow();
	drawPropertiesWindow();
	drawCreationWindow();
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


void GUI::drawInformationWindow()
{
	// set initial size and position for the Information Window
	ImGui::SetNextWindowSize(m_informationWindowSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(m_informationWindowPosition, ImGuiCond_Appearing);
	// set the Information Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that displays all information about the assets in the scene
		// begin the Information window
		ImGui::PushFont(m_boldFont);
		ImGui::Begin("OBJECT INFORMATION", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		drawCamerasInformation();
		ImGui::Text("\n");
		ImGui::Separator();
		drawLightsInformation();
		ImGui::Text("\n");
		ImGui::Separator();
		drawModelsInformation();
		ImGui::Text("\n");

		ImGui::End(); // end the Information window

		if (m_informationWindowJustAppeared)
		{ // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr); // set focus to no window
			m_informationWindowJustAppeared = false; // no longer the first frame
		}
	}
}

void GUI::drawDebugWindow()
{
	// set initial size and position for the Debug Window
	ImGui::SetNextWindowSize(m_debugWindowSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(m_debugWindowPosition, ImGuiCond_Appearing);
	// set the Debug Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that contains debug information and controls
		// begin the Debug window
		ImGui::PushFont(m_boldFont);
		ImGui::Begin("DEBUG", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		// rendering settings section title
		ImGui::PushFont(m_boldFont);
		ImGui::Text("RENDERING INFORMATION");
		ImGui::PopFont();

		// draw the current rendering mode (event-driven or continuous)
		bool eventDriven = Core::GetInstance()->GetEventDriven();
		ImGui::Text("Rendering Mode: %s", eventDriven ? "Event-driven" : "Continuous");

		// rendering settings section title
		ImGui::PushFont(m_boldFont);
		ImGui::Text("RENDERING SETTINGS");
		ImGui::PopFont();

		// draw a checkbox to toggle between event-driven and continuous rendering
		ImGui::Text("Event-Driven Rendering"); // draw the label before the checkbox
		ImGui::SameLine(); // keep the checkbox on the same line as the label
		if (ImGui::Checkbox("##Event-Driven Rendering", &eventDriven)) // '##' to hide the label
		{ // if the checkbox is clicked, toggle the rendering mode
			Core::GetInstance()->SetEventDriven(eventDriven);
		}

		// get the renderer screen debug params from the Core instance
		auto renderer = Core::GetInstance()->GetRenderer();
		auto& params = renderer->GetScreenDebugParams();
		// vector of strings that represent the different debug modes
		std::vector<std::string> debugModes = { "Normal", "Solid Color", "Grid Overlay", "Inverted Colors" };
		int debugModeIndex = static_cast<int>(params.debugMode); // current debug mode index
		bool paramsChanged{ false }; // dirty flag to check if any of the params were changed

		ImGui::Text("Screen Texture Debug Mode");
		ImGui::SameLine(); // keep the combo box on the same line as the label
		// make the combo box take the full width of the window
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		// draw a combo box to select the screen texture debug mode (with the current mode as preview)
		if (ImGui::BeginCombo("##ScreenTextureDebugMode", debugModes[debugModeIndex].c_str(),
							  ImGuiComboFlags_HeightSmall))
		{ // if the combo box is opened
			for (int n{}; n < debugModes.size(); n++)
			{ // iterate through all debug modes
				bool isSelected = (debugModeIndex == n); // check if the current mode is selected
				if (ImGui::Selectable(debugModes[n].c_str(), isSelected))
				{ // if a mode is selected, update the current debug mode locally and in the renderer
					debugModeIndex = n;
					params.debugMode = debugModeIndex;
					paramsChanged = true; // mark the params as changed
				}
				if (isSelected) // whatever mode is selected, set it as the default focus
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo(); // end the combo box
		}

		// depending on the selected debug mode, draw additional controls
		switch (debugModeIndex)
		{
			case 0: // Normal mode
				break; // no additional controls needed
			case 1: // Solid Color mode
			{ // draw a color picker to select the solid color
				ImGui::Text("\tConfiguration");
				ImGui::Text("\t\t"); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				// disable the alpha channel and the inputs (only show an RGB color picker)
				if (ImGui::ColorEdit3("Solid Color", (float*)&params.solidColor,
									  ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha))
				{
					paramsChanged = true; // mark the params as changed
				}
			}
			break;
			case 2: // Grid Overlay mode
			{ // draw controls for each parameter
				ImGui::Text("\tConfiguration");
				ImGui::Text("\t\t"); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100.0f); // set a fixed width for the input field
				if (ImGui::InputInt("Grid Line Count", (int*)&params.gridLineCount,
									1, 10)) // set step values for the input field (normal and fast)
				{ // if the input field is changed, update the number of grid lines
					// clamp the value to a reasonable range [2, 1000]
					if (params.gridLineCount < 2)
						params.gridLineCount = 2;
					else if (params.gridLineCount > 1000)
						params.gridLineCount = 1000;

					// update the number of grid lines in the params
					params.gridLineCount = params.gridLineCount;
					paramsChanged = true; // mark the params as changed
				}
				ImGui::Text("\t\t"); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100.0f); // set a fixed width for the input field
				if (ImGui::InputFloat("Grid Line Thickness", &params.gridLineThickness,
									  0.05f, 0.5f, // set step values for the input field (normal and fast)
									  "%.2f"))
				{ // if the input field is changed, update the grid line thickness
					// clamp the value to a reasonable range [1.0, 10.0]
					params.gridLineThickness = std::clamp(params.gridLineThickness, 1.0f, 10.0f);
					paramsChanged = true; // mark the params as changed
				}

				ImGui::Text("\t\t"); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				// for the background color and the line color,
				// disable the alpha channel and the inputs (only show an RGB color picker)
				if (ImGui::ColorEdit3("Grid Background Color", (float*)&params.gridBgColor,
									  ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha))
					paramsChanged = true; // mark the params as changed
				ImGui::Text("\t\t"); // add some vertical spacing for better visual separation
				ImGui::SameLine();
				if (ImGui::ColorEdit3("Grid Line Color", (float*)&params.gridLineColor,
									  ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha))
					paramsChanged = true; // mark the params as changed
			}
			break;
			case 3: // Inverted Colors mode
				break; // no additional controls needed
			default:
				std::cerr << "[ERROR::GUI::DrawDebugWindow] Unknown screen texture debug mode: "
					<< params.debugMode << std::endl;
				break;
		}

		if (paramsChanged)
		{ // if any of the renderer debug params were changed, apply the changes to the renderer
			renderer->SetScreenDebugParams(params);
		}

		ImGui::End(); // end the Debug window

		if (m_debugWindowJustAppeared)
		{ // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr); // set focus to no window
			m_debugWindowJustAppeared = false; // no longer the first frame
		}
	}
}

void GUI::drawPropertiesWindow()
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	// set initial size and position for the Properties Window
	ImGui::SetNextWindowSize(m_propertiesWindowSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(m_propertiesWindowPosition, ImGuiCond_Appearing);
	// set the Properties Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that allows the user to change the properties of the assets in the scene
		// begin the Properties window
		ImGui::PushFont(m_boldFont);
		ImGui::Begin("PROPERTIES", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		// vector to collect the identifiers of the assets to be removed after the loops
		std::vector<std::uint32_t> assetsToRemove;

		std::for_each(assetManager->GetAssets("LIGHT").begin(),
					  assetManager->GetAssets("LIGHT").end(),
					  [&](const std::shared_ptr<Asset>& asset)
		{ // iterate through all lights in the scene and draw their settings
			// dynamically cast the asset to a Light object
			auto light = dynamic_cast<Light*>(asset.get());

			if (!light) return; // if the light is not valid, skip it

			// draw controls to change the properties of the light
			drawLightControls(light);

			// draw a button to mark the light and its gizmo for removal from the scene
			drawRemoveAssetButton(light, assetsToRemove, light->GetName(), ITEM_WIDTH);

			ImGui::Separator(); // add a separator between lights
		});

		std::for_each(assetManager->GetAssets("MODEL").begin(),
					  assetManager->GetAssets("MODEL").end(),
					  [&](const std::shared_ptr<Asset>& asset)
		{ // iterate through all models in the scene and draw their settings
			// dynamically cast the asset to a Model object
			auto model = dynamic_cast<Model*>(asset.get());

			if (!model) return; // if the model is not valid, skip it

			// if the model is not a gizmo, draw its controls,
			// otherwise skip it, as gizmos are not editable (variable properties handled in light controls)
			if (model->GetGizmoType() == GizmoType::NONE)
			{
				// draw controls to change the properties of the model
				drawModelControls(model);

				// draw a button to mark the model for removal from the scene
				drawRemoveAssetButton(model, assetsToRemove, model->GetName(), ITEM_WIDTH);

				ImGui::Separator(); // add a separator between models
			}
		});

		// remove the assets that were marked for removal
		for (const auto& assetId : assetsToRemove)
			assetManager->RemoveAssetById(assetId); // remove the asset from the asset manager

		ImGui::End(); // end the Properties window

		if (m_propertiesWindowJustAppeared)
		{ // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr); // set focus to no window
			m_propertiesWindowJustAppeared = false; // no longer the first frame
		}
	}
}

void GUI::drawCreationWindow()
{
	// set initial size and position for the Create Window
	ImGui::SetNextWindowSize(m_creationWindowSize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(m_creationWindowPosition, ImGuiCond_Appearing);
	// set the Create Window to be expanded (i.e. not minimized)
	ImGui::SetNextWindowCollapsed(false, ImGuiCond_Appearing);

	{ // show a window that contains buttons to create new objects to the scene
		// begin the Creation window
		ImGui::PushFont(m_boldFont);
		ImGui::Begin("CREATION", nullptr,
					 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing);
		ImGui::PopFont();

		// button to create a new directional light to the scene
		if (ImGui::Button("Create Directional Light", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Directional Light");
			// set random initial values
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			m_newDirection = m_randomizer->GenerateRandomDirection();
		}
		drawCreateDirectionalLightPopup(); // draw the popup to create a new directional light

		// button to create a new point light to the scene
		if (ImGui::Button("Create Point Light", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Point Light");
			// set random initial values
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
		}
		drawCreatePointLightPopup(); // draw the popup to create a new point light

		// button to create a new spotlight to the scene
		if (ImGui::Button("Create Spotlight", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Spotlight");
			// set random initial values
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			m_newDirection = m_randomizer->GenerateRandomDirection();
		}
		drawCreateSpotlightPopup(); // draw the popup to create a new spotlight

		// buttom to create a new cube shape to the scene
		if (ImGui::Button("Create Cube Shape", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			ImGui::OpenPopup("Create Cube Shape");
			// randomize the albedo and position of the new cube shape
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			// initialize the rotation and scale of the new cube shape with default values
			m_newRotation = glm::vec3{ 0.0f };
			m_newScale = glm::vec3{ 1.0f };
		}
		drawCreateCubeShapePopup(); // draw the popup to create a new cube shape

		// button to import a new model from a file
		if (ImGui::Button("Import 3D Model", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{ // if the button is clicked
			// file dialog configuration
			IGFD::FileDialogConfig fileDialogConfig;
			fileDialogConfig.path = "./resources/models"; // initial directory to open the file dialog
			fileDialogConfig.countSelectionMax = 1; // for now, allow only one file to be selected
			fileDialogConfig.flags = ImGuiFileDialogFlags_Modal; // no special flags for the file dialog

			// open a file dialog to select a model file
			ImGuiFileDialog::Instance()->OpenDialog(
				"ChooseFileDlgKey", // unique key for the file dialog
				"Choose 3D Model File", // title of the file dialog
				".obj, .fbx, .dae, .gltf, .glb, .stl, .ply, .3ds, .max", // supported file extensions
				fileDialogConfig // file dialog configuration
			);
		}
		drawImportModelPopup(); // draw the popup for importing a new model

		// create a dummy button to fill the remaining space in the window and test the layout
		ImGui::Button(" ", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)); // create a dummy button with no action

		ImGui::End(); // end the Create Objects window

		if (m_creationWindowJustAppeared)
		{ // if the window just appeared (first frame), prevent it from being focused
			ImGui::SetWindowFocus(nullptr); // set focus to no window
			m_creationWindowJustAppeared = false; // no longer the first frame
		}
	}
}


void GUI::drawCamerasInformation() const
{
	// get the asset manager and scene manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();
	auto& sceneManager = Core::GetInstance()->GetSceneManager();

	ImGui::PushFont(m_boldFont);
	ImGui::Text("CAMERAS");
	ImGui::PopFont();

	// get the number of cameras in the scene
	unsigned int nCameras = assetManager->GetNCameras();
	// display the number of cameras in the scene
	ImGui::PushFont(m_boldFont);
	ImGui::Text("\nNumber of Cameras in the scene: %d", nCameras);
	ImGui::PopFont();

	// display the attributes of each camera in the scene
	ImGui::Text("\nCameras in the scene:");
	std::for_each(assetManager->GetAssets("CAMERA").begin(),
				  assetManager->GetAssets("CAMERA").end(),
				  [&](const std::shared_ptr<Asset>& asset)
	{ // iterate over all cameras in the asset manager and display their attributes
		// dynamically cast the asset to a Camera object
		auto camera = dynamic_cast<Camera*>(asset.get());

		// use PushID to create a unique ID for each camera
		ImGui::PushID(camera->GetName().c_str());

		// display the attributes of the camera
		ImGui::PushFont(m_boldFont);
		ImGui::TextWrapped("\t%s", camera->GetName().c_str());
		ImGui::PopFont();
		ImGui::Text("\t\tPosition: (%.3f, %.3f, %.3f)",
					camera->GetPosition().x,
					camera->GetPosition().y,
					camera->GetPosition().z);
		ImGui::Text("\t\tFront: (%.3f, %.3f, %.3f)",
					camera->GetFront().x,
					camera->GetFront().y,
					camera->GetFront().z);
		ImGui::Text("\t\tUp: (%.3f, %.3f, %.3f)",
					camera->GetUp().x,
					camera->GetUp().y,
					camera->GetUp().z);
		ImGui::Text("\t\tRight: (%.3f, %.3f, %.3f)",
					camera->GetRight().x,
					camera->GetRight().y,
					camera->GetRight().z);
		ImGui::Text("\t\tWorld Up: (%.3f, %.3f, %.3f)",
					camera->GetWorldUp().x,
					camera->GetWorldUp().y,
					camera->GetWorldUp().z);
		ImGui::Text("\t\tYaw: %.3f", camera->GetYaw());
		ImGui::Text("\t\tPitch: %.3f", camera->GetPitch());
		ImGui::Text("\t\tMovement Speed: %.3f", camera->GetMovementSpeed());
		ImGui::Text("\t\tMouse Sensitivity: %.3f", camera->GetMouseSensitivity());
		ImGui::Text("\t\tZoom: %.3f", camera->GetZoom());

		ImGui::PopID(); // use PopID to end the unique ID scope
	});
}

void GUI::drawLightsInformation() const
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	ImGui::PushFont(m_boldFont);
	ImGui::Text("LIGHTS");
	ImGui::PopFont();

	// get the number of lights and of each type of light in the scene
	unsigned int nLights = assetManager->GetNLights();
	unsigned int nDirectionalLights = assetManager->GetNDirectionalLights();
	unsigned int nPointLights = assetManager->GetNPointLights();
	unsigned int nSpotlights = assetManager->GetNSpotlights();
	// display the number of lights and each type of light in the scene
	ImGui::PushFont(m_boldFont);
	ImGui::Text("\nNumber of Lights in the scene: %d", nLights);
	ImGui::PopFont();
	ImGui::Text("\tNumber of Directional Lights in the scene: %d", nDirectionalLights);
	ImGui::Text("\tNumber of Point Lights in the scene: %d", nPointLights);
	ImGui::Text("\tNumber of Spotlights in the scene: %d", nSpotlights);

	// display the attributes of each light in the scene
	ImGui::Text("\nLights in the scene:");
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
				ImGui::PushFont(m_boldFont);
				ImGui::TextWrapped("\t%s", directionalLight->GetName().c_str());
				ImGui::PopFont();
				ImGui::Text("\t\tColor: (%.3f, %.3f, %.3f)",
							directionalLight->GetDiffuse().x,
							directionalLight->GetDiffuse().y,
							directionalLight->GetDiffuse().z);
				ImGui::Text("\t\tPosition: (%.3f, %.3f, %.3f)",
							directionalLight->GetPosition().x,
							directionalLight->GetPosition().y,
							directionalLight->GetPosition().z);
				ImGui::Text("\t\tDirection: (%.3f, %.3f, %.3f)",
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
				ImGui::PushFont(m_boldFont);
				ImGui::TextWrapped("\t%s", pointLight->GetName().c_str());
				ImGui::PopFont();
				ImGui::Text("\t\tColor: (%.3f, %.3f, %.3f)",
							pointLight->GetDiffuse().x,
							pointLight->GetDiffuse().y,
							pointLight->GetDiffuse().z);
				ImGui::Text("\t\tPosition: (%.3f, %.3f, %.3f)",
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
				ImGui::PushFont(m_boldFont);
				ImGui::TextWrapped("\t%s", spotlight->GetName().c_str());
				ImGui::PopFont();
				ImGui::Text("\t\tColor: (%.3f, %.3f, %.3f)",
							spotlight->GetDiffuse().x,
							spotlight->GetDiffuse().y,
							spotlight->GetDiffuse().z);
				ImGui::Text("\t\tPosition: (%.3f, %.3f, %.3f)",
							spotlight->GetPosition().x,
							spotlight->GetPosition().y,
							spotlight->GetPosition().z);
				ImGui::Text("\t\tDirection: (%.3f, %.3f, %.3f)",
							spotlight->GetDirection().x,
							spotlight->GetDirection().y,
							spotlight->GetDirection().z);
				ImGui::Text("\t\tInner cut-off: %.3f", glm::degrees(glm::acos(spotlight->GetInnerCutOff())));
				ImGui::Text("\t\tOuter cut-off: %.3f", glm::degrees(glm::acos(spotlight->GetOuterCutOff())));

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
}

void GUI::drawModelsInformation() const
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	ImGui::PushFont(m_boldFont);
	ImGui::Text("MODELS");
	ImGui::PopFont();

	// get the number of models and of each type of model in the scene
	unsigned int nModels = assetManager->GetNModels();
	unsigned int nAssimpModels = assetManager->GetNAssimpModels();
	unsigned int nShapes = assetManager->GetNShapes();
	// display the number of models and each type of model in the scene
	ImGui::PushFont(m_boldFont);
	ImGui::Text("\nNumber of Models in the scene: %d", nModels);
	ImGui::PopFont();
	ImGui::Text("\tNumber of Assimp models in the scene: %d", nAssimpModels);
	ImGui::Text("\tNumber of Shapes in the scene: %d", nShapes);

	// display the attributes of each model in the scene
	ImGui::Text("\nModels in the scene:");
	std::for_each(assetManager->GetAssets("MODEL").begin(),
				  assetManager->GetAssets("MODEL").end(),
				  [&](const std::shared_ptr<Asset>& asset)
	{ // iterate over all models in the asset manager and display their attributes
		// dynamically cast the asset to a Model object
		auto model = dynamic_cast<Model*>(asset.get());

		ImGui::PushID(model->GetName().c_str()); // use PushID to create a unique ID for each model

		// display the attributes of the model
		ImGui::PushFont(m_boldFont);
		ImGui::TextWrapped("\t%s", model->GetName().c_str());
		ImGui::PopFont();

		if (model->GetModelType() == ModelType::SHAPE)
		{ // only display the albedo color for shapes
			ImGui::Text("\t\tColor: (%.3f, %.3f, %.3f)",
						model->GetAlbedo().x,
						model->GetAlbedo().y,
						model->GetAlbedo().z);
		}
		ImGui::Text("\t\tPosition: (%.3f, %.3f, %.3f)",
					model->GetPosition().x,
					model->GetPosition().y,
					model->GetPosition().z);
		ImGui::Text("\t\tRotation: (%.3f, %.3f, %.3f)",
					model->GetRotationInEulerAngles().x,
					model->GetRotationInEulerAngles().y,
					model->GetRotationInEulerAngles().z);
		ImGui::Text("\t\tScale: (%.3f, %.3f, %.3f)",
					model->GetScale().x,
					model->GetScale().y,
					model->GetScale().z);
		ImGui::Text("\t\tForward: (%.3f, %.3f, %.3f)",
					model->GetForward().x,
					model->GetForward().y,
					model->GetForward().z);

		ImGui::PopID(); // use PopID to end the unique ID scope
	});
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
	ImGui::PushFont(m_boldFont);
	ImGui::TextWrapped("%s", directionalLight->GetName().c_str());
	ImGui::PopFont();

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
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE))
	{ // if the control is used
		directionalLight->SetPosition(pos); // set the new position of the directional light
		directionalLight->SyncGizmoPositionFromLight(); // set the new position of the gizmo
	}

	// retrieve the gizmo of the directional light and its rotation in Euler angles
	auto gizmo = directionalLight->GetGizmo();
	glm::vec3 rotDegrees = gizmo->GetRotationInEulerAngles();
	// create a control for the x, y, and z components of the directional light's rotation
	if (drawVec3Control("Rotation", rotDegrees, false, // is not the scale control
						MIN_ROTATION_VALUE, MAX_ROTATION_VALUE,
						INPUT_FIELD_WIDTH, ROTATION_SPEED, ROTATION_RESET_VALUE))
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
	ImGui::PushFont(m_boldFont);
	ImGui::TextWrapped("%s", pointLight->GetName().c_str());
	ImGui::PopFont();

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
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE))
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
	ImGui::PushFont(m_boldFont);
	ImGui::Text("%s", spotlight->GetName().c_str());
	ImGui::PopFont();

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
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE))
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
						INPUT_FIELD_WIDTH, ROTATION_SPEED, ROTATION_RESET_VALUE))
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
						 INPUT_FIELD_WIDTH, CUTOFF_ANGLES_SPEED, INNER_CUTOFF_RESET_VALUE))
	{ // if the control is used
					// convert back to radians and cosine and
					// set the new inner cut-off angle for the spotlight
		spotlight->SetInnerCutOff(glm::cos(glm::radians(innerCutOff)));
	}
	if (drawFloatControl("Outer", outerCutOff,
						 innerCutOff, MAX_CUTOFF_VALUE, // outerCutOff >= innerCutOff
						 INPUT_FIELD_WIDTH, CUTOFF_ANGLES_SPEED, OUTER_CUTOFF_RESET_VALUE))
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
	ImGui::PushFont(m_boldFont);
	ImGui::TextWrapped("%s", model->GetName().c_str());
	ImGui::PopFont();

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
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE))
	{ // if the control is used
		model->SetPosition(pos); // set the new position of the model
	}

	// get the rotation in Euler angles
	glm::vec3 rotDegrees = model->GetRotationInEulerAngles();
	// create a control for the x, y, and z components of the model's rotation
	if (drawVec3Control("Rotation", rotDegrees, false, // is not the scale control
						MIN_ROTATION_VALUE, MAX_ROTATION_VALUE,
						INPUT_FIELD_WIDTH, ROTATION_SPEED, ROTATION_RESET_VALUE))
	{ // if the control is used
		model->SetRotationInEulerAngles(rotDegrees); // set the new rotation of the model
	}

	// get the scale of the model
	glm::vec3 scale = model->GetScale();
	// if the model is a shape, use a faster speed for scaling, otherwise use the default speed
	float speed = model->GetModelType() == ModelType::SHAPE ? SCALE_SPEED * 5.0f : SCALE_SPEED;
	// create a control for the x, y, and z components of the model's scale
	if (drawVec3Control("Scale", scale, true, // is the scale control
						MIN_SCALE_VALUE, MAX_SCALE_VALUE,
						INPUT_FIELD_WIDTH, speed, SCALE_RESET_VALUE))
	{ // if the control is used
		model->SetScale(scale); // set the new scale of the model
	}

	ImGui::PopID(); // use PopID to end the unique ID scope
}


void GUI::drawCreateDirectionalLightPopup()
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	if (ImGui::BeginPopupModal("Create Directional Light", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(m_boldFont);
		ImGui::Text("Set initial properties for the new Directional Light\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color, position, and direction of the new directional light
		drawColorControl("Color", m_newAlbedo);
		drawVec3Control("Position", m_newPosition, false,
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE);
		drawVec3Control("Direction", m_newDirection, false,
						MIN_DIRECTION_VALUE, MAX_DIRECTION_VALUE,
						INPUT_FIELD_WIDTH, DIRECTION_SPEED, DIRECTION_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new directional light
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new directional light's properties
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			m_newDirection = m_randomizer->GenerateRandomDirection();
		}

		// display a button to add the new directional light
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
				glm::vec3{ 0.1f }, m_newAlbedo, glm::vec3{ 1.0f }, m_newPosition, m_newDirection
			);

			// get the gizmo of the new directional light before creating the light to the engine
			auto newDirectionalLightGizmo = newDirectionalLight->GetGizmo();
			// add the new directional light to the engine
			assetManager->AddAsset(std::move(newDirectionalLight));
			// set the name of the gizmo to include the model number
			newDirectionalLightGizmo->SetName(newDirectionalLightGizmo->GetName() + " (Model " + nModels + ")");
			// create the gizmo of the new directional light to the engine
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

void GUI::drawCreatePointLightPopup()
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	if (ImGui::BeginPopupModal("Create Point Light", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(m_boldFont);
		ImGui::Text("Set initial properties for the new Point Light\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color and position of the new point light
		drawColorControl("Color", m_newAlbedo);
		drawVec3Control("Position", m_newPosition, false,
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new point light
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new point light's properties
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
		}

		ImGui::SameLine();
		// display a button to add the new point light
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Create button is clicked
			// get the number of models and point lights in the scene
			std::string nModels = std::to_string(assetManager->GetNModels());
			std::string nPointLights = std::to_string(assetManager->GetNPointLights());

			// create a new point light with the specified properties
			auto newPointLight = std::make_shared<PointLight>(
				"Point Light " + nPointLights,
				glm::vec3{ 0.1f }, m_newAlbedo, glm::vec3{ 1.0f }, m_newPosition
			);

			// get the gizmo of the new point light before creating the light to the engine
			auto newPointLightGizmo = newPointLight->GetGizmo();
			// add the new point light to the engine
			assetManager->AddAsset(std::move(newPointLight));
			// set the name of the gizmo to include the model number
			newPointLightGizmo->SetName(newPointLightGizmo->GetName() + " (Model " + nModels + ")");
			// create the gizmo of the new point light to the engine
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

void GUI::drawCreateSpotlightPopup()
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	if (ImGui::BeginPopupModal("Create Spotlight", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(m_boldFont);
		ImGui::Text("Set initial properties for the new Spotlight\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color, position, direction, and cut-off angles of the new spotlight
		drawColorControl("Color", m_newAlbedo);
		drawVec3Control("Position", m_newPosition, false,
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE);
		drawVec3Control("Direction", m_newDirection, false,
						DIRECTION_RESET_VALUE, DIRECTION_RESET_VALUE,
						INPUT_FIELD_WIDTH, DIRECTION_SPEED, DIRECTION_RESET_VALUE);
		ImGui::Text("Cut-Off Angles (in degrees):");
		// initialize the inner and outer cut-off angles with default values before drawing the controls
		drawFloatControl("Inner", m_newInnerCutOff,
						 MIN_CUTOFF_VALUE, MAX_CUTOFF_VALUE, // innerCutOff <= outerCutOff
						 INPUT_FIELD_WIDTH, CUTOFF_ANGLES_SPEED, INNER_CUTOFF_RESET_VALUE);
		drawFloatControl("Outer", m_newOuterCutOff,
						 m_newInnerCutOff, MAX_CUTOFF_VALUE, // outerCutOff >= innerCutOff
						 INPUT_FIELD_WIDTH, CUTOFF_ANGLES_SPEED, OUTER_CUTOFF_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new spotlight
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new spotlight's properties
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
			m_newDirection = m_randomizer->GenerateRandomDirection();
			m_newInnerCutOff = m_randomizer->GenerateRandomFloat(
				MIN_CUTOFF_VALUE,
				MAX_CUTOFF_VALUE);
			m_newOuterCutOff = m_randomizer->GenerateRandomFloat(
				m_newInnerCutOff, // ensure outer cut-off is greater than inner cut-off
				MAX_CUTOFF_VALUE);
		}

		// display a button to add the new spotlight
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Create button is clicked
			// get the number of models and spotlights in the scene
			std::string nModels = std::to_string(assetManager->GetNModels());
			std::string nSpotlights = std::to_string(assetManager->GetNSpotlights());

			// create a new spotlight with the specified properties
			auto newSpotlight = std::make_shared<Spotlight>(
				"Spotlight " + nSpotlights,
				glm::vec3{ 0.1f }, m_newAlbedo, glm::vec3{ 1.0f }, m_newPosition, m_newDirection
			);

			// get the gizmo of the new spotlight before creating the light to the engine
			auto newSpotlightGizmo = newSpotlight->GetGizmo();
			// add the new spotlight to the engine
			assetManager->AddAsset(std::move(newSpotlight));
			// set the name of the gizmo to include the model number
			newSpotlightGizmo->SetName(newSpotlightGizmo->GetName() + " (Model " + nModels + ")");
			// create the gizmo of the new spotlight to the engine
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

void GUI::drawCreateCubeShapePopup()
{
	// get the asset manager from the Core instance
	auto& assetManager = Core::GetInstance()->GetAssetManager();

	if (ImGui::BeginPopupModal("Create Cube Shape", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{ // if the popup is open
		// display a message to the user
		ImGui::PushFont(m_boldFont);
		ImGui::Text("Set initial properties for the new Cube Shape\n");
		ImGui::PopFont();

		ImGui::Separator();

		// display controls to set the color, position, and size of the new cube shape
		drawColorControl("Color", m_newAlbedo);
		drawVec3Control("Position", m_newPosition, false,
						MIN_POSITION_VALUE, MAX_POSITION_VALUE,
						INPUT_FIELD_WIDTH, POSITION_SPEED, POSITION_RESET_VALUE);
		drawVec3Control("Rotation", m_newRotation, false, // is not the scale control
						MIN_ROTATION_VALUE, MAX_ROTATION_VALUE,
						INPUT_FIELD_WIDTH, ROTATION_SPEED, ROTATION_RESET_VALUE);
		drawVec3Control("Scale", m_newScale, true,
						MIN_SCALE_VALUE, MAX_SCALE_VALUE,
						INPUT_FIELD_WIDTH, SCALE_SPEED * 5.0f, SCALE_RESET_VALUE);

		ImGui::Separator();

		// display a button to randomize the properties of the new cube shape
		if (ImGui::Button("Randomize", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Randomize button is clicked
			// generate random values for the new cube shape's properties
			m_newAlbedo = m_randomizer->GenerateRandomColor();
			m_newPosition = m_randomizer->GenerateRandomPosition(
				glm::vec3(0.0f), MIN_DISTANCE_TO_ORIGIN, MAX_DISTANCE_TO_ORIGIN);
		}

		// display a button to add the new cube shape
		ImGui::SameLine();
		if (ImGui::Button("Create", ImVec2(POPUP_BUTTON_WIDTH, 0.0f)))
		{ // if the Create button is clicked
			// get the number of models in the scene
			std::string nModels = std::to_string(assetManager->GetNModels());

			// create a cube shape with the specified properties
			auto newCubeShape = std::make_shared<Shape>(
				"Cube (Model " + nModels + ")",
				cubeVerticesVec, cubeIndicesVec,
				m_newAlbedo, m_newPosition, glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f }, m_newScale
			);
			// set the rotation of the new cube shape
			newCubeShape->SetRotationInEulerAngles(m_newRotation);

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

	ImGui::SetNextItemWidth(colorControlWidth); // set the width of the color picker's input fields
	if (ImGui::ColorEdit3("##color", (float*)&color))
	{ // if the color picker is used
		value_changed = true; // set the value_changed flag to true
	}

	ImGui::PopID(); // end the unique ID scope for the label

	return value_changed; // return whether the color has changed
}

bool GUI::drawVec3Control(const std::string& label, glm::vec3& values, bool scaleControls,
						  float minInputFieldValue, float maxInputFieldValue, float inputFieldWidth,
						  float speed, float resetValue, float resetButtonWidth, float resetButtonHeight)
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
	if (ImGui::InputFloat("##x", &values.x,
						  0.0f, 0.0f, // no step buttons
						  "%.3f"))
	{ // if the input field is used
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##upX", ImGuiDir_Up))
	{ // if the up arrow button is pressed
		values.x += speed; // increase the x component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##downX", ImGuiDir_Down))
	{ // if the down arrow button is pressed
		values.x -= speed; // decrease the x component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopButtonRepeat(); // end the repeat mode for the arrow buttons
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(resetButtonWidth, resetButtonHeight)))
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
	if (ImGui::InputFloat("##y", &values.y,
						  0.0f, 0.0f, // no step buttons
						  "%.3f"))
	{ // if the input field is used
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##upY", ImGuiDir_Up))
	{ // if the up arrow button is pressed
		values.y += speed; // increase the y component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##downY", ImGuiDir_Down))
	{ // if the down arrow button is pressed
		values.y -= speed; // decrease the y component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopButtonRepeat(); // end the repeat mode for the arrow buttons
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(resetButtonWidth, resetButtonHeight)))
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
	if (ImGui::InputFloat("##z", &values.z,
						  0.0f, 0.0f, // no step buttons
						  "%.3f"))
	{ // if the input field is used
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##upZ", ImGuiDir_Up))
	{ // if the up arrow button is pressed
		values.z += speed; // increase the z component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	if (ImGui::ArrowButton("##downZ", ImGuiDir_Down))
	{ // if the down arrow button is pressed
		values.z -= speed; // decrease the z component by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopButtonRepeat(); // end the repeat mode for the arrow buttons
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(resetButtonWidth, resetButtonHeight)))
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
						   float minInputFieldValue, float maxInputFieldValue, float inputFieldWidth,
						   float speed, float resetValue, float resetButtonWidth, float resetButtonHeight)
{
	bool value_changed{ false }; // flag to indicate if the value has changed

	ImGui::PushID(label.c_str()); // create a unique ID for the label to avoid conflicts with other controls

	ImGui::Text("%s", label.c_str()); // display the label for the control

	ImGui::SameLine();
	ImGui::SetNextItemWidth(inputFieldWidth); // set the width of the input field
	if (ImGui::InputFloat("##val", &value,
						  0.0f, 0.0f, // no step buttons
						  "%.2f"))
	{ // if the input field is used
		value_changed = true;
	} // set the value_changed flag to true

	// enable repeat mode for the arrow buttons (allows holding down the button to change value continuously)
	ImGui::PushButtonRepeat(true);
	ImGui::SameLine();
	if (ImGui::ArrowButton("##up", ImGuiDir_Up))
	{ // if the up arrow button is pressed
		value += speed; // increase the value by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##down", ImGuiDir_Down))
	{ // if the down arrow button is pressed
		value -= speed; // decrease the value by speed
		value_changed = true; // set the value_changed flag to true
	}
	ImGui::PopButtonRepeat(); // end the repeat mode for the arrow buttons
	ImGui::SameLine();
	if (ImGui::Button("Reset", ImVec2(resetButtonWidth, resetButtonHeight)))
	{ // if the Reset button is pressed
		value = resetValue; // reset the value to the reset value
		value_changed = true; // set the value_changed flag to true
	}

	// clamp the value to the specified range
	value = std::clamp(value, minInputFieldValue, maxInputFieldValue);

	ImGui::PopID(); // end the unique ID scope for the label

	return value_changed;
}

void GUI::drawRemoveAssetButton(Asset* asset, std::vector<uint32_t>& assetsToRemoveIds,
								const std::string& label, float buttonWidth, float buttonHeight)
{
	std::string buttonLabel = "Remove " + label;
	if (ImGui::Button(buttonLabel.c_str(), ImVec2(buttonWidth, buttonHeight)))
	{ // if the button is clicked
		if (asset->GetType() == AssetType::LIGHT)
		{ // if the asset is a light, also add its gizmo to the list of assets to remove
			auto light = static_cast<Light*>(asset); // cast the asset to a Light pointer
			auto& gizmo = light->GetGizmo(); // get the gizmo from the light
			uint32_t gizmoId = gizmo->GetId(); // get the id of the gizmo
			assetsToRemoveIds.push_back(gizmoId); // add the gizmo id to the list of assets to remove
			std::cout << "[INFO::GUI::drawRemoveAssetButton] "
				<< gizmo->GetName() << " with ID " << gizmoId << " marked for removal" << std::endl;
		}
		assetsToRemoveIds.push_back(asset->GetId()); // add the asset id to the list of assets to remove
		std::cout << "[INFO::GUI::drawRemoveAssetButton] "
			<< asset->GetName() << " with ID " << asset->GetId() << " marked for removal" << std::endl;
	}
}
