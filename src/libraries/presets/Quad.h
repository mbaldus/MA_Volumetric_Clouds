//
// Created by Max on 11.05.2018.
//

#ifndef QUAD_H
#define QUAD_H

#include <GL/glew.h>

class Quad
{
public:
    Quad();

    ~Quad();

    void render();

protected:

    GLuint m_vao;
    GLuint m_vbo;
};

#endif //QUAD_H
