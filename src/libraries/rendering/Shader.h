#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>

class Shader
{
public:
   
	// type of the shader (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER, GL_COMPUTE_SHADER)
    Shader(const GLuint &type);

    ~Shader();

    //Loads the shader contents from a string
    void loadFromString(const std::string &sourceString);

    //Loads the the shader contents from a file
    void loadFromFile(const std::string &filename);
    
    //Compile a shader and display any problems if compilation fails.
    void compile(std::string shadername);

    inline GLuint getId()           {return m_id;}  //!< Get the shader id (handle).
    inline std::string getSource()  {return m_source;} //!< get the shader source code as string.

private:
    GLuint m_id;            //!< The unique ID / handle for the shader
    std::string m_typeString; //!< String representation of the shader type (i.e. "Vertex" or such)
    std::string m_source;     //!< The shader source code (i.e. the GLSL code itself)
};


#endif //SHADER_H

    