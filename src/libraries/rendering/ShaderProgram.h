#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

class ShaderProgram
{
public:
  
	ShaderProgram(std::string vertexshader, std::string fragmentshader);
	ShaderProgram(std::string computeshader);

   ~ShaderProgram();

   GLuint getShaderProgramHandle();

    virtual void use();

    ShaderProgram* update(std::string name, bool value);
	ShaderProgram* update(std::string name, int value);
	ShaderProgram* update(std::string name, float value);
	ShaderProgram* update(std::string name, double value);
	ShaderProgram* update(std::string name, glm::ivec2 vector);
	ShaderProgram* update(std::string name, glm::ivec3 vector);
	ShaderProgram* update(std::string name, glm::ivec4 vector);
	ShaderProgram* update(std::string name, glm::vec2 vector);
	ShaderProgram* update(std::string name, glm::vec3 vector);
	ShaderProgram* update(std::string name, glm::vec4 vector);
	ShaderProgram* update(std::string name, glm::mat2 matrix);
	ShaderProgram* update(std::string name, glm::mat3 matrix);
	ShaderProgram* update(std::string name, glm::mat4 matrix);
	ShaderProgram* update(std::string name, std::vector<glm::vec2> vector);
	ShaderProgram* update(std::string name, std::vector<glm::vec3> vector);
	ShaderProgram* update(std::string name, std::vector<glm::vec4> vector);


private:

	void attachShader(Shader shader);
	void link();	//link shaders to a program

	GLuint m_shaderProgramHandle;	//!< handle of the shader program
    GLuint m_shadercount;            //!< the amount of shaders
    std::string m_vertexshader;
    std::string m_fragmentshader;
    std::string m_computeshader;
};


#endif //SHADERPROGRAM_H

    