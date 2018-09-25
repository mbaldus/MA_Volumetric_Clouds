//
// Created by Max on 11.05.2018.
//

#include "Quad.h"

Quad::Quad()
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    float positions[] =
            {
                    -1.0f, -1.0f,
                    -1.0f, 1.0f,
                    1.0f, -1.0f,
                    1.0f, 1.0f
            };

    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8, positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE, 0,0);
    glEnableVertexAttribArray(0);
}

Quad::~Quad()
{
    glDeleteBuffersARB(1, &m_vbo);
}

void Quad::render()
{
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0 ,4);
}