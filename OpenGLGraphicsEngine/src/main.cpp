#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Function prototypes
void processInput(GLFWwindow* window);

// GLFW callback function prototypes
void framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height);
void mouse_callback(GLFWwindow* window, GLdouble xpos, GLdouble ypos);
void scroll_callback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset);

// Screen settings
const GLuint SCR_WIDTH{ 800 };
const GLuint SCR_HEIGHT{ 600 };

// Main function
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

	// GLAD: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//  Configure global OpenGL state
	// --------------------
	// ...

	// Build and compile shader programs
	// ---------------------------------
	// ...

	// Set up buffers and configure vertex attributes
	// ----------------------------------------------
	// ...

	// Load textures
	// -------------
	// ...


	// Render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// Input
		// -----
		processInput(window);

		// Render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// ... render stuff here ...

		// GLFW: swap buffers and poll IO events
		// -------------------------------------
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	// De-allocate all resources once they've outlived their purpose
	// -------------------------------------------------------------
	// ...

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
}

// GLFW callbacks
// --------------
// Resize viewport when window is resized
void framebuffer_size_callback(GLFWwindow* window, GLint width, GLint height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, GLdouble xpos, GLdouble ypos)
{

}

void scroll_callback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset)
{

}
