#pragma once

#include "kigl/kigl.h"

class WaterNoiseGenerator final
{
public:
    WaterNoiseGenerator();

    GLuint generate();

private:
    double smoothNoise(double fov, double x1, double y1, double z1);
    double turbulence(double x, double y, double z, double maxFov);
    void fillDataArray(GLubyte data[]);

private:
};

