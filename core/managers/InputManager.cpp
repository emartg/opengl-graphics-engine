/*
* InputManager.h
* This file implements the InputManager class, which is responsible for handling user input:
* - Mouse cursor position and scroll input
*/

#include "InputManager.h"

#include "../Core.h"
#include "../camera/Camera.h"

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
		if (input == "LMB_HOLD") // left mouse button hold for translation
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
	if (input == "ESC_PRESSED") // if the Escape key is pressed, close the window
	{
		const_cast<Renderer*>(Core::GetInstance()->GetRenderer())->SetWindowShouldClose();
		std::cout << "[INFO::INPUTMANAGER::KeyCallback] Escape key pressed, closing window" << std::endl;
	}
	else if (input == "R_PRESSED") // if the 'R' key is pressed, reset the camera
	{
		Core::GetInstance()->GetSceneManager()->GetCamera()->ResetCamera();
		std::cout << "[INFO::INPUTMANAGER::KeyCallback] Camera reset to default values" << std::endl;
	}
}