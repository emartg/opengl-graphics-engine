/*
* Shader.h
* This file defines the Shader class (a derived class of Node),
* which is used to read, compile, and link shaders to a shader program.
* It also provides methods to set uniform variables in the shader program.
*/

#pragma once

#include "../Node.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader : public Node
{
public:
	// Public Attributes
	// -----------------
	GLuint shaderProgramId;

	// Constructors
	// ------------
	// Constructor without geometry shader
	Shader(const std::string& name,
		   const std::string& vertexPath,
		   const std::string& fragmentPath,
		   const GLboolean deferredCompilation = true);

	// Constructor with geometry shader
	Shader(const std::string& name,
		   const std::string& vertexPath,
		   const std::string& geometryPath,
		   const std::string& fragmentPath,
		   const GLboolean deferredCompilation = true);

	// Destructor
	// ----------
	~Shader() { nShaders--; } // decrement the number of shaders

	// Public Methods
	// --------------
	// Loads the shader
	void Load() override {}

	// Deallocates all the resources of the shader
	void DeallocateResources() override { glDeleteProgram(shaderProgramId); }

	// Does not draw anything by default, as a shader program has no visual representation
	virtual void Draw() const override {}
	// Does not draw anything by default, as a shader program has no visual representation
	virtual void Draw(const Shader& shader) const override {}

	// Compiles the shader from the source code and links it to a shader program
	// Returns true if compilation and linking were successful, false otherwise
	bool Compile();

	// Activates the shader program
	void Use() const;

	// Getters
	GLuint GetShaderProgramId() const { return shaderProgramId; }
	std::string GetVertexPath() const { return m_vertexPath; }
	std::string GetGeometryPath() const { return m_geometryPath; }
	std::string GetFragmentPath() const { return m_fragmentPath; }

	// Uniform setters
	void SetBool(const std::string& name, GLboolean value) const;
	void SetInt(const std::string& name, GLint value) const;
	void SetFloat(const std::string& name, GLfloat value) const;
	void SetVec2(const std::string& name, const glm::vec2& vec) const;
	void SetVec3(const std::string& name, const glm::vec3& vec) const;
	void SetVec4(const std::string& name, const glm::vec4& vec) const;
	void SetMat3(const std::string& name, const glm::mat3& mat) const;
	void SetMat4(const std::string& name, const glm::mat4& mat) const;

	// Public Static Methods
	// ---------------------
	static GLuint GetNModels() { return nShaders; }

private:
	// Private Static Attributes
	// -------------------------
	static GLuint nShaders; // number of shaders in the scene

	// Private Attributes
	// ------------------
	std::string m_vertexPath;   // path to the vertex shader file
	std::string m_geometryPath; // path to the geometry shader file (optional)
	std::string m_fragmentPath; // path to the fragment shader file

	// Private Methods
	// ---------------
	// Utility method to check for shader compilation/linking errors
	// Returns true if there were compilation/linking errors, false otherwise
	bool checkCompileErrors(GLuint shader, std::string type) const;

};