//
// Created by Max on 07.05.2018.
//

#ifndef TEXTURE3D_H
#define TEXTURE3D_H

#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <iostream>
#include "util/stb_image.h"
#include "util/util.h"
#include "Noise.h"

enum class Texture_Mode
{
    perlin_worley,
    erosion_noise,
    cross
};

class Texture3D {
public:
    Texture3D();
    Texture3D(std::string path, int dim);
    Texture3D(int dim, Texture_Mode mode, bool saveToFile=false, float freq = 10);
    ~Texture3D();

    GLuint getHandle();

protected:

    GLuint m_textureHandle;
    int m_width;
    int m_height;
    int m_depth;
    int m_dim;
    bool m_saveToFile;
    Texture_Mode m_mode;
    float m_freq;

    GLuint loadPerlinWorley();
    GLuint loadErosionNoise();
    GLuint loadCross();
    GLuint load(std::string path);
    float remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax);
};


#endif //TEXTURE3D_H
