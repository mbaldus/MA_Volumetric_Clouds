//
// Created by Max on 07.05.2018.
//

#include "Texture3D.h"
#include <fstream>

Texture3D::Texture3D(std::string path, int dim) : m_width(dim), m_height(dim), m_depth(dim)
{
    m_textureHandle = load(path);
}

Texture3D::Texture3D(int dim, Texture_Mode mode, bool saveToFile, float freq) : m_width(dim), m_height(dim),
                                                                                m_depth(dim), m_freq(freq),
                                                                                m_dim(dim), m_mode(mode),
                                                                                m_saveToFile(saveToFile)
{
    if (m_mode == Texture_Mode::perlin_worley)
        m_textureHandle = loadPerlinWorley();
    if (m_mode == Texture_Mode::erosion_noise)
        m_textureHandle = loadErosionNoise();
    if (m_mode == Texture_Mode::cross)
        m_textureHandle = loadCross();
}

Texture3D::~Texture3D()
{
    glDeleteTextures(1, &m_textureHandle);
}

//https://github.com/sebh/TileableVolumeNoise
GLuint Texture3D::loadPerlinWorley()
{
    std::vector<GLubyte> data(m_dim * m_dim * m_dim, 0);
    float frequenceMul[6] = {2, 8, 14, 20, 26, 32};
    int octave = 3;
    float cellCount = 4.f;
    glm::vec3 normFactor = glm::vec3(1.0f / float(m_dim));
    for (auto s = 0; s < m_dim; s++)
    {
        for (auto t = 0; t < m_dim; t++)
        {
            for (auto r = 0; r < m_dim; r++)
            {
                glm::vec3 coord = glm::vec3(s, t, r) * normFactor;

                float freq = m_freq;

                float perlinNoise = Noise::computePerlinNoise(coord, freq, octave);
                float perlinWorleyNoise = 0.0f;
                {
                    float worleyNoise0 = (1.0f - Noise::computeWorleyNoise(coord, cellCount * frequenceMul[0]));
                    float worleyNoise1 = (1.0f - Noise::computeWorleyNoise(coord, cellCount * frequenceMul[1]));
                    float worleyNoise2 = (1.0f - Noise::computeWorleyNoise(coord, cellCount * frequenceMul[2]));

                    float worleyFBM = worleyNoise0 * 0.625f + worleyNoise1 * 0.25f + worleyNoise2 * 0.125f;

                    perlinWorleyNoise = remap(perlinNoise, 0.0, 1.0, worleyFBM, 1.0);
                }
                float worleyNoise0 = (1.0f - Noise::computeWorleyNoise(coord, cellCount * 1));
                float worleyNoise1 = (1.0f - Noise::computeWorleyNoise(coord, cellCount * 2));
                float worleyNoise2 = (1.0f - Noise::computeWorleyNoise(coord, cellCount * 4));
                float worleyNoise3 = (1.0f - Noise::computeWorleyNoise(coord, cellCount * 8));
                float worleyNoise4 = (1.0f - Noise::computeWorleyNoise(coord, cellCount * 16));

                float worleyFBM0 = worleyNoise1 * 0.625f + worleyNoise2 * 0.25f + worleyNoise3 * 0.125f;
                float worleyFBM1 = worleyNoise2 * 0.625f + worleyNoise3 * 0.25f + worleyNoise4 * 0.125f;
                float worleyFBM2 = worleyNoise3 * 0.75f + worleyNoise4 * 0.25f;

                int index = (r * m_dim * m_dim) + t * m_dim + s;

                float value = 0.0;
                {
                    // pack the channels for direct usage in shader
                    float lowFreqFBM = worleyFBM0 * 0.625f + worleyFBM1 * 0.25f + worleyFBM2 * 0.125f;
                    float baseCloud = perlinWorleyNoise;

                    value = remap(baseCloud, -lowFreqFBM, 1.0, 0.0, 1.0);
                    // Saturate
                    value = std::fminf(value, 1.0f);
                    value = std::fmaxf(value, 0.0f);
                }

                data[index] = (GLubyte) (255.f * value);
            }

        }
    }
    if (m_saveToFile)
        saveTextureDataToFile(data, TEXTURES_PATH + std::string("/perlinWorley_") + std::to_string(m_dim) +
                                    std::string(".raw"));

    glEnable(GL_TEXTURE_3D);
    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_3D, m_textureHandle);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R8, m_width, m_height, m_depth);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, m_width, m_height, m_depth, GL_RED, GL_UNSIGNED_BYTE, data.data());
    std::cout << "3D Texture: Perlin Worley Noise loaded" << std::endl;
    checkGLError("Perlin Worley Noise");

    return m_textureHandle;
}

//https://github.com/sebh/TileableVolumeNoise
GLuint Texture3D::loadErosionNoise()
{
    std::vector<GLubyte> data(m_dim * m_dim * m_dim, 0);
    float cellCount = 4;
    glm::vec3 normFactor = glm::vec3(1.0f / float(m_dim));
    for (auto s = 0; s < m_dim; s++)
    {
        for (auto t = 0; t < m_dim; t++)
        {
            for (auto r = 0; r < m_dim; r++)
            {
                glm::vec3 coord = glm::vec3(s, t, r) * normFactor;

                // 3 octaves
                float worleyNoise0 = (1.0f - Noise::computeWorleyNoise(coord, cellCount * 1));
                float worleyNoise1 = (1.0f - Noise::computeWorleyNoise(coord, cellCount * 2));
                float worleyNoise2 = (1.0f - Noise::computeWorleyNoise(coord, cellCount * 4));
                float worleyNoise3 = (1.0f - Noise::computeWorleyNoise(coord, cellCount * 8));
                float worleyFBM0 = worleyNoise0 * 0.625f + worleyNoise1 * 0.25f + worleyNoise2 * 0.125f;
                float worleyFBM1 = worleyNoise1 * 0.625f + worleyNoise2 * 0.25f + worleyNoise3 * 0.125f;
                float worleyFBM2 = worleyNoise2 * 0.75f + worleyNoise3 * 0.25f;

                int index = (r * m_dim * m_dim) + t * m_dim + s;
                float value = 0.0;
                {
                    value = worleyFBM0*0.625f + worleyFBM1*0.25f + worleyFBM2*0.125f;
                }
                data[index] = (GLubyte) (255.f * value);
            }
        }
    }
    if (m_saveToFile)
        saveTextureDataToFile(data, TEXTURES_PATH + std::string("/erosionNoise_") + std::to_string(m_dim) +
                                    std::string(".raw"));

    glEnable(GL_TEXTURE_3D);
    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_3D, m_textureHandle);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R8, m_width, m_height, m_depth);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, m_width, m_height, m_depth, GL_RED, GL_UNSIGNED_BYTE, data.data());

    std::cout << "3D Texture: Erosion Noise loaded" << std::endl;
    checkGLError("Erosion Noise");

    return m_textureHandle;
}


GLuint Texture3D::loadCross()
{
    std::vector<GLubyte> data(m_dim * m_dim * m_dim);
    int range_1 = (m_dim / 2) - (m_dim / 8);
    int range_2 = (m_dim / 2) + (m_dim / 8);
//    std::cout << "Cross ranges: " << range_1 << " , " << range_2 << std::endl;
    glm::vec3 normFactor = glm::vec3(1.0f / float(m_dim));
    for (auto s = 0; s < m_dim; s++)
    {
        for (auto t = 0; t < m_dim; t++)
        {
            for (auto r = 0; r < m_dim; r++)
            {
                int index = (r * m_dim * m_dim) + t * m_dim + s;
                float value = 0.f;

                if ((s > range_1 && s < range_2) && (t > range_1 && t < range_2))
                    value = 0.5f;
                if ((s > range_1 && s < range_2) && (r > range_1 && r < range_2))
                    value = 0.5f;
                if ((t > range_1 && t < range_2) && (r > range_1 && r < range_2))
                    value = 0.5f;

                data[index] = (GLubyte) (255.f * value);
            }

        }
    }

    if (m_saveToFile)
        saveTextureDataToFile(data,
                              TEXTURES_PATH + std::string("/cross") + std::to_string(m_dim) + std::string(".raw"));

    glEnable(GL_TEXTURE_3D);
    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_3D, m_textureHandle);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, m_width, m_height, m_depth);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, m_width, m_height, m_depth, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    std::cout << "3D Texture: Cross loaded" << std::endl;
    checkGLError("3D Texture Cross");

    return m_textureHandle;
}

float Texture3D::remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
    return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

GLuint Texture3D::load(std::string path)
{
    std::vector<GLubyte> data = loadTextureDataFromFile(path);
    glEnable(GL_TEXTURE_3D);
    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_3D, m_textureHandle);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R8, m_width, m_height, m_depth);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, m_width, m_height, m_depth, GL_RED, GL_UNSIGNED_BYTE, data.data());
    std::cout << "3D Texture: " << path << std::endl;
    checkGLError("3D Texture load from file: " + path);

    return m_textureHandle;
}

GLuint Texture3D::getHandle()
{
    return m_textureHandle;
}