#ifndef UTIL_H
#define UTIL_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <GL/glew.h>


float rand_float(float mn, float mx);
void checkGLError(const std::string &where);
void saveTextureDataToFile(std::vector<GLubyte>& data, std::string path);
std::vector<GLubyte> loadTextureDataFromFile(std::string path);
#endif //UTIL_H