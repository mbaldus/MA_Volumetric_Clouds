#include "ShaderProgram.h"
   
ShaderProgram::ShaderProgram(std::string vertexshader,std::string fragmentshader) : m_vertexshader(vertexshader), m_fragmentshader(fragmentshader)
{
	// Initially, we have zero shaders attached to the program
	m_shadercount = 0;

	m_shaderProgramHandle = glCreateProgram();

	//set up shaders
	Shader vertexShader(GL_VERTEX_SHADER);
	vertexShader.loadFromFile(SHADERS_PATH + vertexshader);
	vertexShader.compile(m_vertexshader);

	Shader fragmentShader(GL_FRAGMENT_SHADER);
	fragmentShader.loadFromFile(SHADERS_PATH + fragmentshader);
	fragmentShader.compile(m_fragmentshader);

	//set up shaderprogram
	attachShader(vertexShader);
	attachShader(fragmentShader);
	link();
}

ShaderProgram::ShaderProgram(std::string computeshader) : m_computeshader(computeshader)
{
	m_shadercount = 0;

	m_shaderProgramHandle = glCreateProgram();

	Shader computeShader(GL_COMPUTE_SHADER);
	computeShader.loadFromFile(SHADERS_PATH + computeshader);
	computeShader.compile(m_computeshader);

	attachShader(computeShader);
	link();
}

ShaderProgram::~ShaderProgram()
{

}
    
void ShaderProgram::attachShader(Shader shader)
{	
	// Increment the number of shaders we have associated with the program
	m_shadercount++;
	// Attach the shader to the program
	// Note: We identify the shader by its unique Id value
	glAttachShader( m_shaderProgramHandle, shader.getId());

}

void ShaderProgram::link()
{
	// Perform the linking process
	glLinkProgram(m_shaderProgramHandle);
	// Check the status
	GLint linkStatus;
	glGetProgramiv(m_shaderProgramHandle, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE)
		std::cout << "Shader program linking: "<< m_vertexshader << " " << m_fragmentshader << ":  failed." << std::endl;
//	else
//		std::cout << "Shader program linking: "<< m_vertexshader << " " << m_fragmentshader << ":  OK." << std::endl;
}

GLuint ShaderProgram::getShaderProgramHandle()
{
	return m_shaderProgramHandle;
}

void ShaderProgram::use()
{
	glUseProgram(m_shaderProgramHandle);
}

ShaderProgram* ShaderProgram::update(std::string name, bool value) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniform1i(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), value);
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, int value) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniform1i(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), value);
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, float value) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniform1f(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), value);

	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, double value) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniform1f(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), value);
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, glm::ivec2 vector) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniform2iv(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), 1, glm::value_ptr(vector));
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, glm::ivec3 vector) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniform3iv(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), 1, glm::value_ptr(vector));
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, glm::ivec4 vector) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniform4iv(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), 1, glm::value_ptr(vector));
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, glm::vec2 vector) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniform2fv(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), 1, glm::value_ptr(vector));
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, glm::vec3 vector) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniform3fv(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), 1, glm::value_ptr(vector));
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, glm::vec4 vector) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniform4fv(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), 1, glm::value_ptr(vector));
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, glm::mat2 matrix) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniformMatrix2fv(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, glm::mat3 matrix) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniformMatrix3fv(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, glm::mat4 matrix) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, std::vector<glm::vec2> vector) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniform2fv(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), sizeof(vector), glm::value_ptr((&vector[0])[0]));
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, std::vector<glm::vec3> vector) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniform3fv(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), sizeof(vector), glm::value_ptr((&vector[0])[0]));
	return this;
}

ShaderProgram* ShaderProgram::update(std::string name, std::vector<glm::vec4> vector) 
{
	glUseProgram(m_shaderProgramHandle);
	glUniform4fv(glGetUniformLocation(m_shaderProgramHandle, name.c_str()), sizeof(vector), glm::value_ptr((&vector[0])[0]));
	return this;
}
