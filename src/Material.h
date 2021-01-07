#pragma once

#include <string>
#include <array>
#include <glm/glm.hpp>

#include "Texture.h"
#include "Shader.h"

/*
* https://en.wikipedia.org/wiki/Wavefront_.obj_file
* http://paulbourke.net/dataformats/obj/
* 
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
    Material(std::string& name, unsigned int materialId);
    ~Material();
    int loadTexture(const std::string& baseDir);

    void prepare(Shader* shader);
    void bind(Shader* shader);
 
public:
    const std::string name;
    const unsigned int materialId;

    std::string materialDir;

    // The specular color is declared using Ks, and weighted using the specular exponent Ns.
    // ranges between 0 and 1000
    float ns = 0.0f;

    // The ambient color of the material is declared using Ka. 
    glm::vec3 ka;

    // Similarly, the diffuse color is declared using Kd.
    glm::vec3 kd = { -1, -1, -1 };

    // The specular color is declared using Ks, and weighted using the specular exponent Ns.
    glm::vec3 ks;

    // Ke/map_Ke     # emissive
    glm::vec3 ke;

    // A material can also have an optical density for its surface. This is also known as index of refraction.
    float ni = 0.0f;

    // Materials can be transparent. This is referred to as being dissolved. Unlike real transparency, 
    // the result does not depend upon the thickness of the object. 
    // A value of 1.0 for "d" is the default and means fully opaque, as does a value of 0.0 for "Tr". 
    // Dissolve works on all illumination models.
    float d = 0.0f;

    // Multiple illumination models are available, per material
    // 0. Color on and Ambient off
    // 1. Color on and Ambient on
    // 2. Highlight on
    // 3. Reflection on and Ray trace on
    // 4. Transparency: Glass on, Reflection : Ray trace on
    // 5. Reflection : Fresnel on and Ray trace on
    // 6. Transparency : Refraction on, Reflection : Fresnel offand Ray trace on
    // 7. Transparency : Refraction on, Reflection : Fresnel onand Ray trace on
    // 8. Reflection on and Ray trace off
    // 9. Transparency : Glass on, Reflection : Ray trace off
    // 10. Casts shadows onto invisible surfaces
    int illum = 0;

    std::string map_kd;

    Texture* texture;
};

