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
Camera camera(glm::vec3(4.25f, 2.5f, 4.25f), glm::vec3(0.0f, 1.0f, 0.0f), -135.0f, -24.0f);
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

	glBindVertexArray(cubeVAO);

	// load data into vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(GLfloat), cubeVertices.data(), GL_STATIC_DRAW);

	// load data into index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndices.size() * sizeof(GLuint), cubeIndices.data(), GL_STATIC_DRAW);

	// set the vertex attribute pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), static_cast<GLvoid*>(nullptr));
	glEnableVertexAttribArray(0);

	// unbind buffers and vertex array
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Load textures
	// -------------
	// ...

	// Shader configuration
	// --------------------
	cubeShader.Use();
	cubeShader.SetVec3("albedo", glm::vec3(0.5f, 0.0f, 0.0f)); // red


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
		glDrawElements(GL_TRIANGLES, cubeIndices.size(), GL_UNSIGNED_INT, nullptr);
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

	// Camera controls (WASD)
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// GLFW callbacks
// --------------
void framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, GLdouble xposIn, GLdouble yposIn)
{
	GLfloat xpos = static_cast<GLfloat>(xposIn);
	GLfloat ypos = static_cast<GLfloat>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = GL_FALSE;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset)
{
	camera.ProcessMouseScroll(static_cast<GLfloat>(yoffset));
}
