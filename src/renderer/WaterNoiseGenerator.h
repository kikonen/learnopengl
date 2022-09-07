#pragma once

#include "ki/GL.h"

class WaterNoiseGenerator final
{
public:
    WaterNoiseGenerator();

    GLuint generate();

private:
    double smoothNoise(double zoom, double x1, double y1, double z1);
    double turbulence(double x, double y, double z, double maxZoom);
    void fillDataArray(GLubyte data[]);

private:
};

