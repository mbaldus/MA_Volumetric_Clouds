#include "Texture.h"

Texture::Texture(std::string path)
{
    m_textureHandle = loadFromImage(path);
}

Texture::Texture(int width, int height, float freq, float scale) : m_width(width), m_height(height), m_noiseFreq(freq),
                                                                   m_noiseScale(scale)
{
    m_textureHandle = loadNoise();
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_textureHandle);
}

GLuint Texture::getHandle()
{
    return m_textureHandle;
}

GLuint Texture::loadFromImage(std::string path)
{
    int bytesPerPixel = 0;

    unsigned char* data = stbi_load(path.c_str(), &m_width, &m_height, &bytesPerPixel, 0);

    //flip image vertically
    unsigned char* s = data;
    for (int y = 0; y < m_height / 2; y++)
    {
        unsigned char* e = data + (m_height - y - 1) * m_width * bytesPerPixel;
        for (int x = 0; x < m_width * bytesPerPixel; x++)
        {
            unsigned char temp = *s;
            *s = *e;
            *e = temp;
            s++;
            e++;
        }
    }

    //send image data to the new texture
    if (bytesPerPixel < 3)
    {
        printf("ERROR: Unable to loadFromImage texture image %s\n", path);
        return -1;
    } else
    {
        setTexture(m_width, m_height, bytesPerPixel, data);
    }
    std::cout << "2D Texture " << path <<" loaded" << std::endl;
    //stbi_image_free(data);	//keep copy e.g. for CPU ray tracing
    glGenerateMipmap(GL_TEXTURE_2D);
    return m_textureHandle;
}

GLuint Texture::loadNoise()
{
    GLubyte* data = new GLubyte[m_width * m_height * 4];

    float xFactor = 1.0f / (m_width - 1);
    float yFactor = 1.0f / (m_height - 1);

    for (auto row = 0; row < m_height; row++)
    {
        for (auto col = 0; col < m_width; col++)
        {
            float x = xFactor * col;
            float y = yFactor * row;
            float sum = 0.0f;
            float freq = m_noiseFreq;
            float scale = m_noiseScale;

            for(auto oct =0; oct <4; oct++)
            {
                glm::vec2 p(x * freq, y*freq);
                float perlin = glm::perlin(p) / scale;
                sum += perlin;
                float result = (sum + 1.0f)/2.0f;

                data[((row*m_width+col)*4)+oct] = (GLubyte)(result*255.0f);
                freq*=2.0f;
                scale*=m_noiseScale;
            }
        }
    }

    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA8,m_width,m_height);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,m_width,m_height,GL_RGBA,GL_UNSIGNED_BYTE,data);
    checkGLError("noise2");
    delete [] data;

    return m_textureHandle;
}

void Texture::setTexture(int width, int height, int bytesPerPixel, unsigned char* data)
{
    m_width = width;
    m_height = height;
    m_bytesPerPixel = bytesPerPixel;
    m_data = data;

    glGenTextures(1, &m_textureHandle);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


    glBindTexture(GL_TEXTURE_2D, m_textureHandle);

    if (m_bytesPerPixel == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_data);
    } else if (m_bytesPerPixel == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data);
    } else
    {
        printf("RESOLVED: Unknown format for bytes per pixel in texture, changed to 4\n");
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data);
    }
    glGenerateMipmap(GL_TEXTURE_2D);
}
