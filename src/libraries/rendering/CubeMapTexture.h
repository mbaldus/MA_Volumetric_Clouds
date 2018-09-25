//
// Created by Max on 07.05.2018.
//

#ifndef CUBEMAPTEXTURE_H
#define CUBEMAPTEXTURE_H

#include <GL/glew.h>
#include <vector>
#include <string>
#include <iostream>
#include "Util/stb_image.h"

class CubeMapTexture {
public:
    CubeMapTexture();
    CubeMapTexture(std::vector<std::string>& faces);
    ~CubeMapTexture();

    GLuint getHandle();

    GLuint load(std::vector<std::string>& faces);

protected:
    GLuint m_textureHandle;
    int m_width;
    int m_height;
    int m_bytesPerPixel;
    unsigned char *m_data;
};


#endif //CUBEMAPTEXTURE_H
