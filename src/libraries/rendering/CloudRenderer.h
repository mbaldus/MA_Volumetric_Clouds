//
// Created by Max on 11.05.2018.
//

#ifndef CLOUDRENDERER_H
#define CLOUDRENDERER_H

#include <GL/glew.h>
#include "ShaderProgram.h"
#include "Texture3D.h"
#include "Texture.h"
#include "util/util.h"

#include "presets/Quad.h"
#include "presets/CVK_Sphere.h"

class CloudRenderer
{

public:
    CloudRenderer(GLFWwindow* window,int skyDomeSize);

    ~CloudRenderer();

    ShaderProgram* getPostProcessShaderProgram();
    ShaderProgram* getCloudShaderProgram();

    Texture* getWeatherTex();
    Texture* getCirrusTex();
    Texture* getCirrostratusTex();
    Texture* getCirrocumulusTex();

    GLuint getFBOHandle();
    void setHighAltitudeCloud(int texID);
    void executeMainRenderPass();
    void executePostProcess();
    void execute();

    void cleanup();

protected:
    GLFWwindow* m_window;           // glfw window
    int m_width;                    // render resolution width
    int m_height;                   // render resolution height
    int m_skyDomeSize;              // size of the sphere to render to

    ShaderProgram* m_screenFillingShaderProgram;
    ShaderProgram* m_postProcessShaderProgram;
    ShaderProgram* m_cloudShaderProgram;

    Texture3D* t_perlinWorley128;
    Texture3D* t_erosionNoise32;
    Texture* t_weather;
    Texture* t_cloudHeightTex;
    Texture* t_cirrus;
    Texture* t_cirrostratus;
    Texture* t_cirrocumulus;



    GLuint m_VAO;
    GLuint m_FBOHandle;
    GLuint m_depthRenderBuffer;


    GLuint m_cloudsOutputTexture;         // texture for screenfilling quad
    GLuint m_cloudsAlphaTexture;          // texture for back UVWs
    GLuint m_pastPostProcessTexture;

    Quad* m_screenFillingQuad;
    Sphere* m_skyDome;

    void createTextures();
    void bindTextures();
    void createFBO();

};

#endif //CLOUDRENDERER_H
