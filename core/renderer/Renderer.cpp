/*
* Renderer.cpp
* Implements the Renderer interface, which is responsible for:
* - Initializing OpenGL
* - Creating and managing windows
* - Rendering graphics
* - Managing GUI elements
* - Event polling and buffer swapping
* - Handling input
* - Etc.
* This interface allows for different implementations of renderers, such as GLFWRenderer, SDLRenderer, etc.
*/

#include "Renderer.h"

// Destructor
// ----------
Renderer::~Renderer() = default;

// Public Methods
// --------------
void Renderer::ConfigOpenGL() const
{
	// depth buffer configuration:
	// 1. Enable the depth test
	// 2. Set the depth function to GL_LESS, which is the default depth function,
	//    i.e., discard fragments whose depth value is greater than or equal to 
	//    the current fragment's depth value
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // default depth function (discard fragments behind the current fragment)

	// stencil buffer configuration:
	// 1. Enable the stencil test
	// 2. Set the stencil operation to the default operation, which means that
	//    the stencil value will not be modified by the stencil test whatever the outcome
	//    of the depth and stencil tests is
	// 3. Set the stencil function to pass only if the stencil value is not equal 
	//    to the reference value, which is set to 1 in this case
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

	// blend configuration:
	glEnable(GL_BLEND); // enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // set the blend function to use alpha blending

	// cull configuration:
	glEnable(GL_CULL_FACE); // enable face culling
	glCullFace(GL_BACK); // cull back faces (default OpenGL behavior)
}

void Renderer::ClearBuffers(BufferType bufferType) const
{
	GLbitfield mask = 0; // initialize the mask to zero (no buffers cleared by default)
	switch (bufferType) // determine which buffers to clear based on the buffer type
	{
		case BufferType::COLOR:
			mask |= GL_COLOR_BUFFER_BIT;
			break;
		case BufferType::DEPTH:
			mask |= GL_DEPTH_BUFFER_BIT;
			break;
		case BufferType::STENCIL:
			mask |= GL_STENCIL_BUFFER_BIT;
			break;
		case BufferType::COLOR_DEPTH:
			mask |= GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
			break;
		case BufferType::COLOR_STENCIL:
			mask |= GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
			break;
		case BufferType::DEPTH_STENCIL:
			mask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
			break;
		case BufferType::ALL:
		default:
			mask |= GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
			break;
	}
	glClear(mask); // clear the specified buffers
}

void Renderer::SetViewport(int width, int height) const
{
	glViewport(0, 0, width, height); // set the viewport to the specified width and height
}

void Renderer::SetClearColor(float r, float g, float b, float a) const
{
	glClearColor(r, g, b, a); // set the clear color for the color buffer (alpha defaults to 1.0f)
}