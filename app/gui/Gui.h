/*
* GUI.h
* This file defines the GUI class, which is used to create a graphical user interface
* using the ImGui library.
* The engine will use this class to create a window that will display information about the scene
* and allow the user to interact with it and change certain parameters.
*/

#pragma once

#include <iostream>
#include <memory>
#include <unordered_set>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp> // for glm::decompose
#define GLFW_INCLUDE_NONE // prevent GLFW from including OpenGL headers
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

// Forward declaration of classes to avoid cyclic includes and allow virtual interfaces and pointers
class Node;
class Light;
class DirectionalLight;
class PointLight;
class Spotlight;
class Random;

// Forward declaration of enum class to avoid cyclic includes
enum class NodeType;
enum class LightType;

class GUI
{
public:
	// Constructors
	// ------------
	GUI();

	// Destructor
	// ----------
	~GUI();

	// Public Methods
	// --------------
	// Initializes the GUI with the given GLFW window and GLSL version
	void InitGUI(GLFWwindow* window, const char* glslVersion);
	// Builds the GUI by starting a new ImGui frame and setting up the layout
	void BuildGUI();
	// Renders the GUI by drawing the ImGui windows and handling input events
	void RenderGUI();
	// Shuts down the GUI and cleans up resources
	void ShutdownGUI() const;

private:
	// Private Attributes
	// ------------------
	std::unique_ptr<Random> m_randomizer; // random generator to get random colors, positions, etc.

	// attributes for the new objects to be created
	glm::vec4 m_newAlbedo;
	glm::vec3 m_newPosition, m_newRotation, m_newDirection, m_newScale;
	float m_newInnerCutOff, m_newOuterCutOff;

	// set of node ids that should be auto-opened in the Scene Graph window
	std::unordered_set<std::uint32_t> m_sceneGraphAutoOpenIds;
	// ids to actually force-open this frame (re-armed per selection change)
	std::unordered_set<std::uint32_t> m_sceneGraphPendingOpenIds;
	// last selected id used to refresh the auto-open ids set (allows manual collapsing)
	std::uint32_t m_lastAutoOpenSelectedId{ 0 };

	// parameters for the GUI layout and windows
	// relative widths and heights of the windows relative to the display size
	float m_sceneGraphWindowRelativeWidth, m_sceneGraphWindowRelativeHeight;
	float m_debugWindowRelativeWidth, m_debugWindowRelativeHeight;
	float m_creationWindowRelativeWidth, m_creationWindowRelativeHeight;
	float m_propertiesWindowRelativeWidth, m_propertiesWindowRelativeHeight;
	// offsets the windows from the edges of the display
	float m_sceneGraphWindowXOffset, m_sceneGraphWindowYOffset;
	float m_debugWindowXOffset, m_debugWindowYOffset;
	float m_propertiesWindowXOffset, m_propertiesWindowYOffset;
	float m_creationWindowXOffset, m_creationWindowYOffset;
	// padding of the windows from the edges of the display
	ImVec2 m_windowPositionPadding, m_windowSizePadding;
	// positions and sizes of the windows in the display
	ImVec2 m_sceneGraphWindowPosition, m_debugWindowPosition,
		m_creationWindowPosition, m_propertiesWindowPosition;
	ImVec2 m_sceneGraphWindowSize, m_debugWindowSize,
		m_creationWindowSize, m_propertiesWindowSize;
	// flags for the windows to prevent focus on the first frame (indicating that the window just appeared)
	bool m_sceneGraphWindowJustAppeared, m_debugWindowJustAppeared,
		m_creationWindowJustAppeared, m_propertiesWindowJustAppeared;

	// style attributes for the GUI
	ImFont* m_mediumFont; // medium font for the GUI (default font)
	ImFont* m_boldFont; // bold font for the GUI

	// Private Static Attributes
	// -------------------------
	static bool s_proportionalScaling; // flag for enabling/disabling proportional scaling

	// default values for ImGui widgets
	static constexpr float INPUT_FIELD_WIDTH{ 70.0f }, INPUT_FIELD_HEIGHT{ 20.0f };
	static constexpr float BUTTON_WIDTH{ 55.0f }, BUTTON_HEIGHT{ 22.5f };
	static constexpr float POPUP_BUTTON_WIDTH{ 120.0f }, POPUP_BUTTON_HEIGHT{ 20.0f };
	static constexpr float POPUP_WIDTH{ 400.0f }, POPUP_HEIGHT{ 300.0f };
	static constexpr float FILE_DIALOG_POPUP_WIDTH{ 1000.0f }, FILE_DIALOG_POPUP_HEIGHT{ 600.0f };
	static constexpr float ERROR_POPUP_WIDTH{ 400.0f }, ERROR_POPUP_HEIGHT{ 150.0f };
	// default values for ImGui controls
	static constexpr float MIN_POSITION_VALUE{ -100.0f }, MAX_POSITION_VALUE{ 100.0f };
	static constexpr float MIN_ROTATION_VALUE{ -360.0f }, MAX_ROTATION_VALUE{ 360.0f };
	static constexpr float MIN_DIRECTION_VALUE{ -1.0f }, MAX_DIRECTION_VALUE{ 1.0f };
	static constexpr float MIN_SCALE_VALUE{ 0.001f }, MAX_SCALE_VALUE{ 100.0f };
	static constexpr float MIN_CUTOFF_VALUE{ 0.0f }, MAX_CUTOFF_VALUE{ 45.0f };
	static constexpr float POSITION_SPEED{ 0.15f }, ROTATION_SPEED{ 1.0f },
		DIRECTION_SPEED{ 0.01f }, SCALE_SPEED{ 0.002f }, CUTOFF_ANGLES_SPEED{ 0.25f };
	static constexpr float POSITION_RESET_VALUE{ 0.0f }, ROTATION_RESET_VALUE{ 0.0f },
		DIRECTION_RESET_VALUE{ 0.0f }, SCALE_RESET_VALUE{ 1.0f },
		INNER_CUTOFF_RESET_VALUE{ 12.5f }, OUTER_CUTOFF_RESET_VALUE{ 32.5f };
	// default values for distances from the origin
	static constexpr float MIN_DISTANCE_TO_ORIGIN{ 4.0f }, MAX_DISTANCE_TO_ORIGIN{ 15.0f };

	// Private Methods
	// ---------------
	// Initializes the GUI layout attributes that do not depend on the display size
	void initGUILayoutAttributes();

	// Configures the ImGui style (fonts, colors, etc.)
	void configureGUIStyle();

	// Starts a new ImGui frame and configures the ImGui style
	void beginGUIFrame() const;
	// Sets the GUI layout attributes based on the current display size
	void configureGUILayout();
	// Draws the GUI windows
	void drawGUIWindows();
	// Handles input events for ImGui
	void handleImGuiInput() const;
	// Resets all GUI windows to their default layout for the current display size
	void resetGUILayout();

	// Draws the Scene Graph Window with a hierarchical tree view of all nodes
	void drawSceneGraphWindow();
	// Draws the Properties Window with controls for the objects in the scene
	void drawPropertiesWindow();
	// Draws the Creation Window with buttons to add new objects to the scene
	void drawCreationWindow();
	// Draws the Debug Window with debug information and scene and GUI controls
	void drawDebugWindow();

	// Recursively draws a node and its children in the scene graph tree
	void drawNodeTreeRecursive(const std::shared_ptr<Node>& node);
	// Handles node selection logic (single selection only)
	void handleNodeSelection(std::uint32_t nodeId);
	// Updates the set of node ids that should be auto-opened in the scene graph window
	void updateSceneGraphAutoOpenSet();

	// Draws controls for a light. Depending on the type of light, 
	// it will call dynamically cast to the appropriate light type and draw the corresponding controls
	void drawLightControls(Light* light);
	// Draws controls for a directional light
	void drawDirectionalLightControls(DirectionalLight* directionalLight);
	// Draws controls for a point light
	void drawPointLightControls(PointLight* pointLight);
	// Draws controls for a spotlight
	void drawSpotlightControls(Spotlight* spotlight);
	// Draws controls for a model
	void drawModelControls(Node* model);

	// Draws a remove button for an node and adds its Id to the vector of nodes marked for removal
	void drawRemoveNodeButton(Node* node, std::vector<std::uint32_t>& nodesToRemoveIds,
							  const std::string& label = "Remove",
							  float buttonWidth = BUTTON_WIDTH, float buttonHeight = BUTTON_HEIGHT);

	// Draws a pop-up modal window to create a new directional light
	void drawCreateDirectionalLightPopup();
	// Draws a pop-up modal window to create a new point light
	void drawCreatePointLightPopup();
	// Draws a pop-up modal window to create a new spotlight
	void drawCreateSpotlightPopup();
	// Draws a pop-up modal window to create a new plane shape
	void drawCreatePlaneShapePopup();
	// Draws a pop-up modal window to create a new cube shape
	void drawCreateCubeShapePopup();
	// Draws a pop-up modal window to import a model from a file
	void drawImportModelPopup();
	// Draws a pop-up modal window to import a skybox from a folder (6 textures)
	void drawImportSkyboxPopup();

	// Creates a color picker with sliders for RGB components
	// and returns true if the color was changed
	bool drawColorControl(const std::string& label, glm::vec3& color, bool showLabel = true,
						  float colorPickerWidth = ImGui::GetContentRegionAvail().x);
	// Creates a color picker with sliders for RGBA components
	// and returns true if the color was changed
	bool drawColorControl(const std::string& label, glm::vec4& color, bool showLabel = true,
						  float colorPickerWidth = ImGui::GetContentRegionAvail().x);
	// Creates a 3-component vector control with input fields and buttons
	// and returns true if any of the components were changed
	bool drawVec3Control(const std::string& label, glm::vec3& values, bool scaleControls,
						 float minInputFieldValue, float maxInputFieldValue,
						 float inputFieldWidth = INPUT_FIELD_WIDTH,
						 float speed = 0.1f, float resetValue = 0.0f,
						 float resetButtonWidth = BUTTON_WIDTH, float resetButtonHeight = BUTTON_HEIGHT);
	// Draws a float control with an input field and arrow buttons
	// and returns true if the value was changed
	bool drawFloatControl(const std::string& label, float& value,
						  float minInputFieldValue, float maxInputFieldValue,
						  float inputFieldWidth = INPUT_FIELD_WIDTH,
						  float speed = 0.1f, float resetValue = 0.0f,
						  float resetButtonWidth = BUTTON_WIDTH, float resetButtonHeight = BUTTON_HEIGHT);

};