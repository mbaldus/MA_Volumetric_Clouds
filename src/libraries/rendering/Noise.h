//
// Created by Max on 29.05.2018.
//

#ifndef MA_VOLUMETRIC_CLOUD_RENDERING_NOISE_H
#define MA_VOLUMETRIC_CLOUD_RENDERING_NOISE_H

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

class Noise
{
public:
    static float computeWorleyNoise(glm::vec3 &p, float cellCount);

    static float computePerlinNoise(glm::vec3 &p, float frequency, int octaveCount);

private:

    static float hash(float n);

    static float noise(glm::vec3 &x);

    static float cells(glm::vec3 &p, float numCells);
};


#endif //MA_VOLUMETRIC_CLOUD_RENDERING_NOISE_H
