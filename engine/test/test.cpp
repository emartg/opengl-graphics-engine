#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "../camera/Camera.h"
#include "../shader/Shader.h"
#include "../model/Model.h"
#include "../model/Shape.h"

#include "CUBE.h"

// Function prototypes
void processInput(GLFWwindow* window);

// GLFW callback function prototypes
void framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height);
void mouse_callback(GLFWwindow* window, GLdouble xposIn, GLdouble yposIn);
void scroll_callback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset);

// Screen settings
const GLuint SCR_WIDTH{ 800 };
const GLuint SCR_HEIGHT{ 600 };

// Camera settings
glm::vec3 initialCameraPosition{ 4.25f, 2.5f, 4.25f }, initialCameraUp{ 0.0f, 1.0f, 0.0f };
GLfloat initialCameraYaw{ -135.0f }, initialCameraPitch{ -24.0f };
Camera camera(initialCameraPosition, initialCameraUp, initialCameraYaw, initialCameraPitch);
GLfloat lastX{ SCR_WIDTH / 2.0f };
GLfloat lastY{ SCR_HEIGHT / 2.0f };
GLboolean firstMouse{ true };

// Time settings
GLfloat deltaTime{ 0.0f }; // time between current frame and last frame
GLfloat lastFrame{ 0.0f };


int main()
{
	// GLFW: initialize and configure
	// ------------------------------
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation
	// ---------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Engine", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// GLFW: register callbacks
	// ------------------------
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// GLFW: other configurations
	// --------------------------
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // cursor is visible but not confined to the window

	// GLAD: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//  Configure global OpenGL state
	// ------------------------------
	glEnable(GL_DEPTH_TEST);

	// Build and compile shader programs
	// ---------------------------------
	Shader cubeShader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");

	// Load models
	// -----------
	// Cube model
	Model* cubeModel = new Shape(cubeVertices, cubeNormals, cubeTexCoords, cubeIndices);

	// Shader configuration
	// --------------------
	cubeShader.Use();

	// vertex shader uniforms
	cubeShader.SetVec3("lightPos", glm::vec3(-1.0f, 2.0f, 2.0f));

	// fragment shader uniforms
	cubeShader.SetVec3("albedo", glm::vec3(0.5f, 0.0f, 0.0f));			// red
	cubeShader.SetFloat("shininess", 32.0f);
	cubeShader.SetVec3("light.ambient", glm::vec3(0.1f, 0.1f, 0.1f));	// low influence of ambient light
	cubeShader.SetVec3("light.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));	// slightly higher influence of diffuse light
	cubeShader.SetVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));	// full influence of specular light
	cubeShader.SetFloat("light.constant", 1.0f);						// constant attenuation term for a distance of 50
	cubeShader.SetFloat("light.linear", 0.09f);							// linear attenuation term for a distance of 50
	cubeShader.SetFloat("light.quadratic", 0.032f);						// quadratic attenuation term for a distance of 50

	// Render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// Per-frame time logic
		// --------------------
		GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Input
		// -----
		processInput(window);

		// Render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// activate shader
		cubeShader.Use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.GetZoom()),
												static_cast<GLfloat>(SCR_WIDTH) / static_cast<GLfloat>(SCR_HEIGHT), 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		cubeShader.SetMat4("projection", projection);
		cubeShader.SetMat4("view", view);
		// world transformation
		glm::mat4 model{ 1.0f };
		cubeShader.SetMat4("model", model);

		// render the model
		cubeModel->Draw(cubeShader);

		// GLFW: swap buffers and poll IO events
		// -------------------------------------
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	// De-allocate all resources once they've outlived their purpose
	// -------------------------------------------------------------
	glDeleteProgram(cubeShader.ID);

	delete cubeModel;

	// GLFW: terminate, clearing all previously allocated GLFW resources
	// -----------------------------------------------------------------
	glfwTerminate();

	return 0;
}

// Function definitions
// --------------------
// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
	// Close window on ESC
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Reset camera position and rotation on R
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		camera = Camera(initialCameraPosition, initialCameraUp, initialCameraYaw, initialCameraPitch);
}

// GLFW callbacks
// --------------
void framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, GLdouble xposIn, GLdouble yposIn)
{
	static GLboolean rightMouseButtonPressed{ false };
	static GLboolean leftMouseButtonPressed{ false };

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		rightMouseButtonPressed = true;
		leftMouseButtonPressed = false;
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		leftMouseButtonPressed = true;
		rightMouseButtonPressed = false;
	}
	else
	{
		rightMouseButtonPressed = false;
		leftMouseButtonPressed = false;
	}

	GLfloat xpos = static_cast<GLfloat>(xposIn);
	GLfloat ypos = static_cast<GLfloat>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	if (rightMouseButtonPressed) // the right mouse button is used to rotate the camera
		camera.ProcessMouseRotation(xoffset, yoffset);
	else if (leftMouseButtonPressed) // the left mouse button is used to translate the camera in 2D
		camera.ProcessMouseTranslation(xoffset, yoffset, 0.025f);
}

void scroll_callback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset)
{
	camera.ProcessMouseScroll(static_cast<GLfloat>(yoffset), 2.5f);
}
