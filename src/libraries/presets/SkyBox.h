//
// Created by Max on 07.05.2018.
//

#ifndef SKYBOX_H
#define SKYBOX_H

#include <GL/glew.h>
#include "rendering/ShaderProgram.h"
#include "rendering/CubeMapTexture.h"
#include "CVK_Cube.h"

class SkyBox {

public:

    SkyBox();
    ~SkyBox();

    void updateCamera(glm::mat4 &view, glm::mat4 &projection);

    void render(glm::mat4& view);

protected:
    ShaderProgram* m_shaderprogram;
    Cube* m_skyBoxCube;

    CubeMapTexture* m_cubeMapTexture;
    void init();
};


#endif SKYBOX_H
