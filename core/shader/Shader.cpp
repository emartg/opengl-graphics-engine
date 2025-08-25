/*
* Shader.cpp
* This file implements the Shader class (a derived class of Asset),
* which is used to read, compile, and link shaders to a shader program.
* It also provides methods to set uniform variables in the shader program.
*/

#include "Shader.h"

// Private Static Attributes
// -------------------------
GLuint Shader::nShaders{}; // initialize the number of shaders in the scene to 0

// Constructors
// ------------
Shader::Shader(const std::string& name,
			   const std::string& vertexPath,
			   const std::string& fragmentPath,
			   const GLboolean deferredCompilation)
	: Asset(name, AssetType::SHADER),
	shaderProgramId{}, // initialize the ID to 0
	m_vertexPath{ vertexPath },
	m_geometryPath{ "" },
	m_fragmentPath{ fragmentPath }
{
	nShaders++; // increment the number of shaders

	// compile the shader if deferredCompilation is set to false
	if (!deferredCompilation) Compile();
}

Shader::Shader(const std::string& name,
			   const std::string& vertexPath,
			   const std::string& geometryPath,
			   const std::string& fragmentPath,
			   const GLboolean deferredCompilation)
	: Asset(name, AssetType::SHADER),
	shaderProgramId{}, // initialize the ID to 0
	m_vertexPath{ vertexPath },
	m_geometryPath{ geometryPath },
	m_fragmentPath{ fragmentPath }
{
	nShaders++; // increment the number of shaders

	// compile the shader if deferredCompilation is set to false
	if (!deferredCompilation) Compile();
}

// Public Methods
// --------------
bool Shader::Compile()
{
	// retrieve the vertex/geometry/fragment source code from the file paths
	std::string vertexCode;
	std::string geometryCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream gShaderFile;
	std::ifstream fShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	if (!m_geometryPath.empty())
		gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		vShaderFile.open(m_vertexPath);
		if (!m_geometryPath.empty())
			gShaderFile.open(m_geometryPath);
		fShaderFile.open(m_fragmentPath);
		// read files' buffer contents into streams
		std::stringstream vShaderStream, gShaderStream, fShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		if (!m_geometryPath.empty())
			gShaderStream << gShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		if (!m_geometryPath.empty())
			gShaderFile.close();
		fShaderFile.close();
		// convert streams into strings
		vertexCode = vShaderStream.str();
		if (!m_geometryPath.empty())
			geometryCode = gShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cerr << "[ERROR::SHADER::Compile] Shader file reading error: " << e.what() << std::endl;
		return false; // if file reading failed, return false
	}
	// convert strings to GLchar pointers
	const GLchar* vShaderCode = vertexCode.c_str();
	const GLchar* gShaderCode = nullptr;
	if (!m_geometryPath.empty())
		gShaderCode = geometryCode.c_str();
	const GLchar* fShaderCode = fragmentCode.c_str();

	// compile shaders and create shader program
	GLuint vertex, geometry{}, fragment;
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	if (!checkCompileErrors(vertex, "VERTEX"))
		return false; // if compilation of the vertex shader failed, return false
	// geometry shader (if provided)
	if (!m_geometryPath.empty())
	{
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		if (!checkCompileErrors(geometry, "GEOMETRY"))
			return false; // if compilation of the geometry shader failed, return false
	}
	// fragment shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	if (!checkCompileErrors(fragment, "FRAGMENT"))
		return false; // if compilation of the fragment shader failed, return false
	// shader program
	shaderProgramId = glCreateProgram();
	glAttachShader(shaderProgramId, vertex);
	if (!m_geometryPath.empty())
		glAttachShader(shaderProgramId, geometry);
	glAttachShader(shaderProgramId, fragment);
	glLinkProgram(shaderProgramId);
	if (!checkCompileErrors(shaderProgramId, "PROGRAM"))
		return false; // if linking the program failed, return false

	// delete the shaders as they're linked into our program and are no longer needed
	glDeleteShader(vertex);
	if (!m_geometryPath.empty())
		glDeleteShader(geometry);
	glDeleteShader(fragment);

	std::cout << "[SUCCESS::SHADER::Compile] Shader compiled and linked successfully: "
		<< GetName() << std::endl;
	return true; // return true if compilation and linking were successful
}

void Shader::Use() const
{
	glUseProgram(shaderProgramId);
}

void Shader::SetBool(const std::string& name, GLboolean value) const
{
	glUniform1i(glGetUniformLocation(shaderProgramId, name.c_str()), (GLint)value);
}

void Shader::SetInt(const std::string& name, GLint value) const
{
	glUniform1i(glGetUniformLocation(shaderProgramId, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, GLfloat value) const
{
	glUniform1f(glGetUniformLocation(shaderProgramId, name.c_str()), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& vec) const
{
	glUniform2fv(glGetUniformLocation(shaderProgramId, name.c_str()), 1, glm::value_ptr(vec));
}

void Shader::SetVec3(const std::string& name, const glm::vec3& vec) const
{
	glUniform3fv(glGetUniformLocation(shaderProgramId, name.c_str()), 1, glm::value_ptr(vec));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramId, name.c_str()), 1, GL_FALSE,
					   glm::value_ptr(mat));
}

// Private Methods
// ---------------
bool Shader::checkCompileErrors(GLuint shader, std::string type) const
{
	GLint success; // variable to store the success status of the compilation/linking process
	GLchar infoLog[1024]; // buffer to store the info log

	if (type != "PROGRAM")
	{ // if the type is not PROGRAM, it means we are checking a shader, so we check for compilation errors
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{ // if compilation failed, retrieve the info log, print it, and return false
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cerr << "[ERROR::SHADER::Compile] Shader compilation error of type: " << type << "\n"
				<< infoLog << "\n-------------------------------------------------------------" << std::endl;
			return false;
		}
	}
	else
	{ // if the type is PROGRAM, it means we are checking a program, so we check for linking errors
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{ // if linking the program failed, retrieve the info log, print it, and return false
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cerr << "[ERROR::SHADER::Compile] Shader linking error of type: " << type << "\n"
				<< infoLog << "\n-------------------------------------------------------------" << std::endl;
			return false;
		}
	}

	return true; // return true if no errors occurred during the compilation/linking process of the shader
}