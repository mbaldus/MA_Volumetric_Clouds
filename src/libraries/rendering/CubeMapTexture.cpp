//
// Created by Max on 07.05.2018.
//

#include "CubeMapTexture.h"
CubeMapTexture::CubeMapTexture()
{
    std::vector<std::string> faces{
            TEXTURES_PATH + std::string("/skybox/sb_right.jpg"),
            TEXTURES_PATH + std::string("/skybox/sb_left.jpg"),
            TEXTURES_PATH + std::string("/skybox/sb_top.jpg"),
            TEXTURES_PATH + std::string("/skybox/sb_bottom.jpg"),
            TEXTURES_PATH + std::string("/skybox/sb_back.jpg"),
            TEXTURES_PATH + std::string("/skybox/sb_front.jpg")
    };
    m_textureHandle = load(faces);
}

CubeMapTexture::CubeMapTexture(std::vector<std::string>& faces)
{
    m_textureHandle = load(faces);
}

CubeMapTexture::~CubeMapTexture()
{
    glDeleteTextures(1, &m_textureHandle);
}

GLuint CubeMapTexture::getHandle()
{
    return m_textureHandle;
}

GLuint CubeMapTexture::load(std::vector<std::string>& faces)
{

    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureHandle);
    for(auto i = 0; i < 6; i++)
    {
        int bytesPerPixel = 0;
        unsigned char *data = stbi_load(faces[i].c_str(), &m_width, &m_height, &bytesPerPixel,0);

        if (bytesPerPixel <3)
        {
            printf("ERROR: Unable to loadFromImage texture image %s\n", faces[i].c_str());
            return -1;
        }
        else if(bytesPerPixel == 3)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, m_width, m_height,0,  GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else if (bytesPerPixel == 4)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, m_width, m_height,0,  GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return m_textureHandle;
}
