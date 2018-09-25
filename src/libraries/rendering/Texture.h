#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <iostream>
#include "util/stb_image.h"
#include "util/util.h"

class Texture
{
public:
   Texture(std::string path);
   Texture(int width, int height, float freq, float scale); 	//for 2D-Noise
   ~Texture();

   GLuint getHandle();

   void setTexture(int width, int height, int bytesPerPixel, unsigned char *data);
protected:

	GLuint m_textureHandle;
	int m_width;
	int m_height;
	float m_noiseFreq;
	float m_noiseScale;
	int m_bytesPerPixel;
	unsigned char *m_data;

	GLuint loadFromImage(std::string path);
	GLuint loadNoise();
};


#endif //TEXTURE_H
