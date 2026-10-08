/*
 * Shader.h
 * This file defines the Shader class (a derived class of Node),
 * which is used to read, compile, and link shaders to a shader program.
 * It also provides methods to set uniform variables in the shader program.
 */

#pragma once

#include "../Node.h"
#include "Shader_Preprocessor.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader : public Node
{
public:
	// Public Attributes
	// -----------------
	GLuint shader_program_id;

	// Constructors
	// ------------
	// Constructor without geometry shader
	Shader(
		const std::string& name,
		const std::string& vertex_path,
		const std::string& fragment_path,
		const GLboolean    deferred_compilation = true);

	// Constructor with geometry shader
	Shader(
		const std::string& name,
		const std::string& vertex_path,
		const std::string& geometry_path,
		const std::string& fragment_path,
		const GLboolean    deferred_compilation = true);

	// Destructor
	// ----------
	~Shader() = default;

	// Public Methods
	// --------------
	// Loads the shader
	void load() override {}

	// Deallocates all the resources of the shader
	void deallocate_resources() override { glDeleteProgram(shader_program_id); }

	// Does not draw anything by default, as a shader program has no visual representation
	virtual void draw() const override {}
	// Does not draw anything by default, as a shader program has no visual representation
	virtual void draw(const Shader& shader) const override {}

	// Compiles the shader from the source code and links it to a shader program
	// Returns true if compilation and linking were successful, false otherwise
	bool compile();

	// Activates the shader program
	void use() const;

	// Getters
	GLuint      get_shader_program_id() const { return shader_program_id; }
	std::string get_vertex_shader_path() const { return vertex_shader_path; }
	std::string get_geometry_shader_path() const { return geometry_shader_path; }
	std::string get_fragment_shader_path() const { return fragment_shader_path; }

	// Uniform setters
	void set_bool(const std::string& name, GLboolean value) const;
	void set_int(const std::string& name, GLint value) const;
	void set_float(const std::string& name, GLfloat value) const;
	void set_vec2(const std::string& name, const glm::vec2& vec) const;
	void set_vec3(const std::string& name, const glm::vec3& vec) const;
	void set_vec4(const std::string& name, const glm::vec4& vec) const;
	void set_mat3(const std::string& name, const glm::mat3& mat) const;
	void set_mat4(const std::string& name, const glm::mat4& mat) const;

private:
	// Private Attributes
	// ------------------
	std::string vertex_shader_path;   // path to the vertex shader file
	std::string geometry_shader_path; // path to the geometry shader file (optional)
	std::string fragment_shader_path; // path to the fragment shader file

	// Private Methods
	// ---------------
	// Utility method to check for shader compilation/linking errors, printing the files of the source
	// (used to identify the file of each error message) if there are compilation errors.
	// Returns true if there were no compilation/linking errors, false otherwise
	bool check_compilation_linking_errors(GLuint shader, std::string type, const std::vector<std::filesystem::path>& files) const;
};
