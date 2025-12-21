/*
* InputManager.h
* This file implements the InputManager class, which is responsible for handling user input:
* - Mouse cursor position and scroll input
*/

#include "InputManager.h"

#include "NodeManager.h"
#include "SceneManager.h"
#include "SelectionManager.h"
#include "../Core.h"
#include "../camera/Camera.h"
#include "../renderer/Renderer.h"

// Constructor
// -----------
InputManager::InputManager()
	: m_lastMouseX{}, m_lastMouseY{}, m_mouseSensitivity{ 1.0f },
	m_firstMouse{ true }, m_cameraControlEnabled{ false }
{}

// Destructor
// ----------
InputManager::~InputManager() = default;

// Public Methods
// --------------
// Callback Methods
void InputManager::CursorPosCallback(GLdouble xpos, GLdouble ypos, std::string input)
{
	if (m_firstMouse) // to prevent the camera from jumping to the mouse position on the first input
	{
		m_lastMouseX = xpos;
		m_lastMouseY = ypos;
		m_firstMouse = false;
	}

	GLfloat xoffset = xpos - m_lastMouseX;
	GLfloat yoffset = m_lastMouseY - ypos; // reversed since y-coordinates range from bottom to top
	m_lastMouseX = xpos;
	m_lastMouseY = ypos;

	if (m_cameraControlEnabled) // only process mouse input if camera control is enabled
	{
		if (input == "MIDDLE_HOLD") // middle mouse button hold for translation
			Core::GetInstance()->GetSceneManager()->GetCamera()->ProcessMouseTranslation(xoffset, yoffset);
		else if (input == "RMB_HOLD") // right mouse button hold for rotation
			Core::GetInstance()->GetSceneManager()->GetCamera()->ProcessMouseRotation(xoffset, yoffset);
	}
}

void InputManager::ScrollCallback(GLdouble xoffset, GLdouble yoffset)
{
	if (m_cameraControlEnabled) // only process scroll input if camera control is enabled
		Core::GetInstance()->GetSceneManager()->GetCamera()->ProcessMouseScroll(yoffset, 2.5f);
}

void InputManager::KeyCallback(std::string input)
{
	auto core = Core::GetInstance(); // get the Core instance
	if (!core) // ensure the Core instance is valid before proceeding
	{ // if the Core instance is null, print an error message and return
		std::cerr << "[ERROR::INPUTMANAGER::KeyCallback] Core instance is null" << std::endl;
		return;
	}
	auto renderer = core->GetRenderer(); // get the Renderer instance
	if (!renderer) // ensure the Renderer instance is valid before proceeding
	{ // if the Renderer instance is null, print an error message and return
		std::cerr << "[ERROR::INPUTMANAGER::KeyCallback] Renderer instance is null" << std::endl;
		return;
	}

	// handle key inputs
	if (input == "ESC_PRESSED")
	{ // if the 'Escape' key is pressed, set the window to close
		const_cast<Renderer*>(renderer)->SetWindowShouldClose();
		std::cout << "[INFO::INPUTMANAGER::KeyCallback] Escape key pressed, closing window..." << std::endl;
	}
	else if (input == "DEL_PRESSED")
	{ // if the 'Delete' key is pressed, delete the selected node (if any) via the selection manager
		auto& selectionMgr = core->GetSelectionManager();
		auto& nodeManager = core->GetNodeManager();

		// get the currently selected node id
		std::uint32_t selectedNodeId = selectionMgr->GetSelectedNodeId();
		if (selectedNodeId != 0)
		{ // if a node is selected, delete it
			auto node = nodeManager->GetNodeById(selectedNodeId);
			if (node)
			{ // if the node exists, log an info message and delete it via the selection manager
				std::cout << "[INFO::INPUTMANAGER::KeyCallback] Delete key pressed, deleting selected node "
					<< node->GetName() << " (ID " << selectedNodeId << ")" << std::endl;

				selectionMgr->DeleteSelected(nodeManager.get());
			}
		}
	}
	else
	{ // if the input is not recognized, print an error message
		std::cerr << "[ERROR::INPUTMANAGER::KeyCallback] Unknown key input: " << input << std::endl;
	}
}