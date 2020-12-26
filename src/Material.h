#pragma once

#include <string>
#include "Texture.h"

/*
# Blender MTL File : 'texture_cube.blend'
# Material Count : 1

newmtl steel
Ns 225.000000
Ka 1.000000 1.000000 1.000000
Kd 0.800000 0.800000 0.800000
Ks 0.500000 0.500000 0.500000
Ke 0.000000 0.000000 0.000000
Ni 1.450000
d 1.000000
illum 2
map_Kd texture_cube.jpg
*/
class Material
{
public:
    Material();
    ~Material();
public:
    std::string name;
    float ns = 0.0f;
    float ka[3];
    float kd[3];
    float ks[3];
    float ke[3];
    float ni[3];
    float d = 0.0f;
    int illum = 0;
    std::string map_kd;

    Texture* texture;
};

