//
// Created by Max on 11.05.2018.
//

#include "CloudRenderer.h"

CloudRenderer::CloudRenderer(GLFWwindow* window, int skyDomeSize) : m_window(window), m_skyDomeSize(skyDomeSize)
{
    glfwGetFramebufferSize(window, &m_width, &m_height);

    m_screenFillingQuad = new Quad;
    m_cloudShaderProgram = new ShaderProgram("/clouds.vert", "/clouds.frag");
    m_cloudShaderProgramID = m_cloudShaderProgram->getShaderProgramHandle();
    m_postProcessShaderProgram = new ShaderProgram("/postProcess.vert", "/postProcess.frag");
    m_postProcessShaderProgramID = m_postProcessShaderProgram->getShaderProgramHandle();
    m_screenFillingShaderProgram = new ShaderProgram("/fullscreen.vert", "/fullscreen.frag");
    m_screenFillingShaderProgramID = m_screenFillingShaderProgram->getShaderProgramHandle();
    checkGLError("Shader initialization");

    m_skyDome = new Sphere(float(m_skyDomeSize));

    // generate textures
    createTextures();
    // bind generated textures to shaderprogram
    bindTextures();
    // create FBO for first renderpass
    createFBO();
}

CloudRenderer::~CloudRenderer()
{

}

void CloudRenderer::createTextures()
{
   // load 3D textures from .raw files
   t_perlinWorley128 = new Texture3D(TEXTURES_PATH + std::string("/perlinWorleyNoise_128.raw"), 128);
   t_erosionNoise32 =new Texture3D(TEXTURES_PATH + std::string("/erosionNoise_32.raw"), 32);
   // load 2D textures from .jpg files
   t_weather = new Texture(TEXTURES_PATH + std::string("/coverage.jpg"));
   t_cloudHeightTex = new Texture(TEXTURES_PATH + std::string("/perlinNoise2D.jpg"));
   t_cirrus = new Texture(TEXTURES_PATH + std::string("/cirrus.jpg"));
   t_cirrostratus = new Texture(TEXTURES_PATH + std::string("/cirrostratus.jpg"));
   t_cirrocumulus= new Texture(TEXTURES_PATH + std::string("/cirrocumulus.jpg"));
   checkGLError("Texture initialization");
}

void CloudRenderer::bindTextures()
{
    m_cloudShaderProgram->use();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, t_perlinWorley128->getHandle());
    glUniform1i(glGetUniformLocation(m_cloudShaderProgram->getShaderProgramHandle(), "u_perlinWorleyBase"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, t_erosionNoise32->getHandle());
    glUniform1i(glGetUniformLocation(m_cloudShaderProgram->getShaderProgramHandle(), "u_erosionNoise"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, t_weather->getHandle());
    glUniform1i(glGetUniformLocation(m_cloudShaderProgram->getShaderProgramHandle(), "u_weatherTexture"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, t_cloudHeightTex->getHandle());
    glUniform1i(glGetUniformLocation(m_cloudShaderProgram->getShaderProgramHandle(), "u_cloudHeightTex"), 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, t_cirrus->getHandle());
    glUniform1i(glGetUniformLocation(m_cloudShaderProgram->getShaderProgramHandle(), "u_highAltCloudTex"), 5);
}

void CloudRenderer::setHighAltitudeCloud(int texID)
{
    glActiveTexture(GL_TEXTURE5);
    switch (texID)
    {
        case (0):
            glBindTexture(GL_TEXTURE_2D, t_cirrus->getHandle());
            break;
        case (1):
            glBindTexture(GL_TEXTURE_2D, t_cirrostratus->getHandle());
            break;
        case (2):
            glBindTexture(GL_TEXTURE_2D, t_cirrocumulus->getHandle());
            break;
    }
    glUniform1i(glGetUniformLocation(m_cloudShaderProgram->getShaderProgramHandle(), "u_highAltCloudTex"), 5);
}

void CloudRenderer::createFBO()
{
    // VAO and FBO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenFramebuffers(1, &m_FBOHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBOHandle);

    //FBO for outputtexture
    glGenTextures(1, &m_cloudsOutputTexture);
    glBindTexture(GL_TEXTURE_2D, m_cloudsOutputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_cloudsOutputTexture, 0);

    //Cloud Alpha Texture
    glGenTextures(1, &m_cloudsAlphaTexture);
    glBindTexture(GL_TEXTURE_2D, m_cloudsAlphaTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_cloudsAlphaTexture, 0);

    glGenTextures(1, &m_pastPostProcessTexture);
    glBindTexture(GL_TEXTURE_2D, m_pastPostProcessTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_pastPostProcessTexture, 0);

    //Renderbuffer
    glGenRenderbuffers(1, &m_depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::string what = std::string(__FILE__) + " " + std::string(__FUNCTION__) + std::string(": FBO not complete");
        throw std::runtime_error(what);
    }
    GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, DrawBuffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

void CloudRenderer::executeMainRenderPass()
{
    //bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBOHandle);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_cloudShaderProgram->use();
    m_skyDome->render();
}

void CloudRenderer::executePostProcess()
{
    //bind screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //execute postprocessing offscreen
    glDisable(GL_DEPTH_TEST);
    m_postProcessShaderProgram->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_cloudsOutputTexture);
    glUniform1i(glGetUniformLocation(m_postProcessShaderProgram->getShaderProgramHandle(), "cloudOutputTexture"), 0);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_cloudsAlphaTexture);
    glUniform1i(glGetUniformLocation(m_postProcessShaderProgram->getShaderProgramHandle(), "alphaTexture"), 6);
    m_screenFillingQuad->render();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void CloudRenderer::execute()
{
    //bind screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //render framebuffer on screenfilling quad
    glDisable(GL_DEPTH_TEST);
    m_screenFillingShaderProgram->use();
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, m_pastPostProcessTexture);
    glUniform1i(glGetUniformLocation(m_screenFillingShaderProgram->getShaderProgramHandle(), "postProcessOutput"), 7);
    m_screenFillingQuad->render();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void CloudRenderer::cleanup()
{
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &m_VAO);

    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &m_cloudsOutputTexture);
    glDeleteTextures(1, &m_cloudsAlphaTexture);
    glDeleteTextures(1, &m_pastPostProcessTexture);
    glDeleteFramebuffers(1, &m_FBOHandle);

    delete m_screenFillingQuad;
    delete m_screenFillingShaderProgram;
    delete m_postProcessShaderProgram;
    delete m_cloudShaderProgram;

    delete t_perlinWorley128;
    delete t_erosionNoise32;
    delete t_weather;
    delete t_cloudHeightTex;
    delete t_cirrus;
    delete t_cirrostratus;
    delete t_cirrocumulus;
}

GLuint CloudRenderer::getFBOHandle()
{
    return m_FBOHandle;
}

ShaderProgram* CloudRenderer::getCloudShaderProgram()
{
    return m_cloudShaderProgram;
}

ShaderProgram* CloudRenderer::getPostProcessShaderProgram()
{
    return m_postProcessShaderProgram;
}

Texture* CloudRenderer::getWeatherTex()
{
    return t_weather;
}

Texture* CloudRenderer::getCirrusTex()
{
    return t_cirrus;
}

Texture* CloudRenderer::getCirrostratusTex()
{
    return t_cirrostratus;
}

Texture* CloudRenderer::getCirrocumulusTex()
{
    return t_cirrocumulus;
}
