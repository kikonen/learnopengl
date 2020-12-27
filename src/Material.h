#pragma once

#include <string>
#include <array>
#include <glm/glm.hpp>

#include "Texture.h"
#include "Shader.h"

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
    Material(std::string& name);
    ~Material();
    int loadTexture(const std::string& baseDir);

    void prepare(Shader* shader);
    void bind();
public:
    std::string materialDir;
    std::string name;

    float ns = 0.0f;
    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    glm::vec3 ke;
    float ni = 0.0f;
    float d = 0.0f;
    int illum = 0;
    std::string map_kd;

    Texture* texture;
};

