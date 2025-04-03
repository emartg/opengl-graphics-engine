/*
* Shader.h
* This file defines the Shader class (a derived class of Asset),
* which is used to read, compile, and link shaders.
*/

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Asset.h"

class Shader : public Asset
{
public:
	// Public Attributes
	// -----------------
	GLuint ID; // the shader program ID

	// Constructors
	// ------------
	// Constructor that reads and builds the shader with a vertex and fragment shader
	Shader(const std::string& name,
		   const GLchar* vertexPath, const GLchar* fragmentPath);
	// Constructor that reads and builds the shader with a geometry shader 
	// in addition to the vertex and fragment shaders
	Shader(const std::string& name,
		   const GLchar* vertexPath, const GLchar* geometryPath, const GLchar* fragmentPath);

	// Public Methods
	// --------------
	// Loads the shader
	void Load() override {}

	// Deallocates all the resources of the shader
	void DeallocateResources() override { glDeleteProgram(ID); }

	// Activates the shader program
	void Use();

	// Uniform setters
	void SetBool(const std::string& name, GLboolean value) const;
	void SetInt(const std::string& name, GLint value) const;
	void SetFloat(const std::string& name, GLfloat value) const;
	void SetVec3(const std::string& name, const glm::vec3& vec) const;
	void SetMat4(const std::string& name, const glm::mat4& mat) const;

private:
	// Private Methods
	// ---------------
	// Utility method to check for shader compilation/linking errors
	void checkCompileErrors(GLuint shader, std::string type);

};