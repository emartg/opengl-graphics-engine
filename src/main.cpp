#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "camera/Camera.h"
#include "shaders/Shader.h"

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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
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
	Shader cubeShader("src/shaders/shader.vert", "src/shaders/shader.frag");

	// Set up buffers and configure vertex attributes
	// ----------------------------------------------
	GLuint cubeVAO, cubeVBO, cubeEBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);

	// interleave the vertex data so that it is laid out as follows: vertex, normal, texture coordinate
	std::vector<GLfloat> interleavedData;
	for (size_t i = 0; i < cubeVertices.size() / 3; ++i) {
		interleavedData.push_back(cubeVertices[i * 3]);
		interleavedData.push_back(cubeVertices[i * 3 + 1]);
		interleavedData.push_back(cubeVertices[i * 3 + 2]);
		interleavedData.push_back(cubeNormals[i * 3]);
		interleavedData.push_back(cubeNormals[i * 3 + 1]);
		interleavedData.push_back(cubeNormals[i * 3 + 2]);
		interleavedData.push_back(cubeTexCoords[i * 2]);
		interleavedData.push_back(cubeTexCoords[i * 2 + 1]);
	}

	glBindVertexArray(cubeVAO);

	// load data into vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, interleavedData.size() * sizeof(GLfloat), interleavedData.data(), GL_STATIC_DRAW);

	// set the vertex attribute pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0); // vertex coordinates
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))); // normal coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat))); // texture coordinates
	glEnableVertexAttribArray(2);

	// load index data and configure element buffer object
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndices.size() * sizeof(GLuint), cubeIndices.data(), GL_STATIC_DRAW);

	// unbind the objects
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Load textures
	// -------------
	// ...

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

		// render the cube
		glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
		glDrawElements(GL_TRIANGLES, cubeIndices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// GLFW: swap buffers and poll IO events
		// -------------------------------------
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	// De-allocate all resources once they've outlived their purpose
	// -------------------------------------------------------------
	glDeleteProgram(cubeShader.ID);

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);

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
