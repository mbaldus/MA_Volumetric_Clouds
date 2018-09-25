#include "util.h"

float rand_float(float mn, float mx)
{
    float r = rand() / (float) RAND_MAX;
    return mn + (mx-mn)*r;
}

void checkGLError(const std::string &when) {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cout << "gl Error in " << when << ": " << err << std::endl;
	}
}

void saveTextureDataToFile(std::vector<GLubyte>& data, std::string path)
{
	std::ofstream outFile(path, std::ios::out | std::ofstream::binary);
	std::copy(data.begin(),data.end(),std::ostreambuf_iterator<char>(outFile));
}

std::vector<GLubyte> loadTextureDataFromFile(std::string path)
{
	std::vector<GLubyte> outputData;
    std::ifstream inputFile(path, std::ios::in | std::ifstream::binary);
    std::istreambuf_iterator<char> iterator(inputFile);
    std::istreambuf_iterator<char> end{};
    std::copy(iterator, end, std::back_inserter(outputData));

    return outputData;
}
