//
// Created by Max on 07.05.2018.
//

#include "SkyBox.h"


SkyBox::SkyBox()
{
    init();
}

SkyBox::~SkyBox()
{
    delete m_shaderprogram;
    delete m_skyBoxCube;
    delete m_cubeMapTexture;
}

void SkyBox::init()
{
    m_shaderprogram = new ShaderProgram("/cubeMap.vert", "/cubeMap.frag");
    m_cubeMapTexture = new CubeMapTexture();
    m_skyBoxCube = new Cube(50.f);
    m_shaderprogram->use();
    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeMapTexture->getHandle());
    glUniform1i(glGetUniformLocation(m_shaderprogram->getShaderProgramHandle(), "skybox"),15);
}

void SkyBox::render(glm::mat4& view)
{
    glDepthFunc(GL_LEQUAL);
    m_shaderprogram->use();
    m_shaderprogram->update("view", view);
    m_skyBoxCube->render();
    glDepthFunc(GL_LESS);
}

void SkyBox::updateCamera(glm::mat4 &view, glm::mat4 &projection)
{
    m_shaderprogram->use();
    m_shaderprogram->update("view", view);
    m_shaderprogram->update("projection", projection);
}