#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "Texture.h"
#include "Shader.h"
#include "UBO.h"

struct BoundTexture {
    Texture* texture = nullptr;
    int unitIndex = -1;

    void bind()
    {
        if (!texture) return;
        //glActiveTexture(GL_TEXTURE0 + unitIndex);
        //glBindTexture(GL_TEXTURE_2D, texture->textureID);
        glBindTextures(unitIndex, 1, &texture->textureID);
    }

    void unbind()
    {
        KI_GL_UNBIND(glBindTexture(GL_TEXTURE_2D, 0));
    }
};

enum MaterialType {
    basic,
    gold,
    silver,
    bronze,
};

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
class Material final
{
public:
    Material(const std::string& name);
    ~Material();
    int loadTextures(const std::string& baseDir);

    void prepare();
    void bindArray(Shader* shader, int index, bool bindTextureIDs);
    //void bind(Shader* shader);

    MaterialUBO toUBO();

    static Material* createDefaultMaterial();

    static Material* createMaterial(MaterialType type);
private:
    BoundTexture* loadTexture(const std::string& baseDir, const std::string& name);

public:
    const std::string name;
    unsigned int materialIndex = 0;

    bool used = false;

    std::string materialDir = { "" };

    TextureSpec textureSpec;

    int pattern = -1;
    float reflection = 0.f;
    float refraction = 0.f;

    float fogRatio = 1.0f;

    float tiling = 1.0f;

    BoundTexture* diffuseTex = nullptr;
    BoundTexture* emissionTex = nullptr;
    BoundTexture* specularTex = nullptr;
    BoundTexture* normalMapTex = nullptr;

    std::vector<BoundTexture*> textures;

    // The specular color is declared using Ks, and weighted using the specular exponent Ns.
    // ranges between 0 and 1000
    float ns = 0.0f;

    // The ambient color of the material is declared using Ka. 
    glm::vec4 ka = { 0.f, 0.f, 0.f, 1.f };

    // Similarly, the diffuse color is declared using Kd.
    glm::vec4 kd = { 0.f, 0.f, 0.f, 1.f };
    std::string map_kd = "";

    // The specular color is declared using Ks, and weighted using the specular exponent Ns.
    glm::vec4 ks = { 0.f, 0.f, 0.f, 1.f };
    std::string map_ks = "";

    // Ke/map_Ke     # emissive
    glm::vec4 ke = { 0.f, 0.f, 0.f, 1.f };
    std::string map_ke = "";

    // some implementations use 'map_bump' instead of 'bump' below
    // bump map(which by default uses luminance channel of the image)
    // bump lemur_bump.tga
    std::string map_bump = "";

    // A material can also have an optical density for its surface. This is also known as index of refraction.
    float ni = 0.0f;

    // Materials can be transparent. This is referred to as being dissolved. Unlike real transparency, 
    // the result does not depend upon the thickness of the object. 
    // A value of 1.0 for "d" is the default and means fully opaque, as does a value of 0.0 for "Tr". 
    // Dissolve works on all illumination models.
    float d = 1.0f;

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

private:
    bool loaded = false;
};

