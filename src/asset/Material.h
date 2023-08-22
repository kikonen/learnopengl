#pragma once

#include <string>
#include <vector>
#include <array>
#include <glm/glm.hpp>

#include "Texture.h"

#include "MaterialSSBO.h"


enum class BasicMaterial {
    basic,
    gold,
    silver,
    bronze,
    highlight,
    selection
};

enum class MaterialType {
    asset,
    model,
    texture,
    sprite
};

constexpr int MATERIAL_DIFFUSE_IDX = 0;
constexpr int MATERIAL_EMISSION_IDX = 1;
constexpr int MATERIAL_SPECULAR_IDX = 2;
constexpr int MATERIAL_NORMAL_MAP_IDX = 3;
constexpr int MATERIAL_DUDV_MAP_IDX = 4;
constexpr int MATERIAL_HEIGHT_MAP_IDX = 5;
constexpr int MATERIAL_NOISE_MAP_IDX = 6;
constexpr int MATERIAL_ROUGHNESS_MAP_IDX = 7;
constexpr int MATERIAL_METALNESS_MAP_IDX = 8;
constexpr int MATERIAL_OPACITY_MAP_IDX = 9;
constexpr int MATERIAL_TEXTURE_COUNT = MATERIAL_OPACITY_MAP_IDX + 1;

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
map_Kd texture_cube_512.png
*/
struct Material final
{
public:
    struct BoundTexture {
        Texture* texture = nullptr;
        int m_texIndex = -1;
        GLuint64 m_handle = 0;

        bool valid() {
            return texture;
        }
    };

public:
    Material();

    Material(const Material&) = default;
    Material(Material&&) = default;
    Material& operator=(const Material&) = default;
    Material& operator=(Material&&) = default;

    ~Material();

    void loadTextures(const Assets& assets);

    bool hasTex(int index) const;

    void prepare(const Assets& assets);

    const MaterialSSBO toSSBO() const;

    const float getRefractionRatio() const{
        return refractionRatio[1] != 0 ? refractionRatio[0] / refractionRatio[1] : refractionRatio[0];
    }

    static Material createMaterial(BasicMaterial type);

    static Material* find(
        const std::string& name,
        std::vector<Material>& materials);

    static Material* findID(
        const int objectID,
        std::vector<Material>& materials);

    static const Material* findID(
        const int objectID,
        const std::vector<Material>& materials);

    const std::string getTexturePath(
        const Assets& assets,
        const std::string& textureName);

private:
    std::string resolveBaseDir(const Assets& assets);

    void loadTexture(
        const Assets& assets,
        int idx,
        const std::string& name);

public:
    int m_objectID;

    std::string m_name;
    std::string m_path;

    MaterialType m_type{ MaterialType::model };

    bool m_default = false;
    bool m_used = false;
    mutable int m_registeredIndex = -1;

    TextureSpec textureSpec;

    int pattern = -1;
    float reflection = 0.f;
    float refraction = 0.f;
    glm::vec2 refractionRatio{ 0 };

    float tilingX = 1.0f;
    float tilingY = 1.0f;

    std::array<BoundTexture, MATERIAL_TEXTURE_COUNT> m_textures;

    // The specular color is declared using Ks, and weighted using the specular exponent Ns.
    // ranges between 0 and 1000
    float ns = 0.0f;

    // The ambient color of the material is declared using Ka.
    // HACK KI a bit ambient even if forgotten
    // => otherwise total blackness shall happen in few cases
    glm::vec3 ka { 0.01f, 0.01f, 0.01f };

    // Similarly, the diffuse color is declared using Kd.
    glm::vec4 kd { 0.f, 0.f, 0.f, 1.f };
    std::string map_kd;

    // The specular color is declared using Ks, and weighted using the specular exponent Ns.
    glm::vec3 ks { 0.f, 0.f, 0.f };
    std::string map_ks;

    // Ke/map_Ke     # emissive
    glm::vec4 ke { 0.f, 0.f, 0.f, 0.f };
    std::string map_ke;

    // some implementations use 'map_bump' instead of 'bump' below
    // bump map(which by default uses luminance channel of the image)
    // bump lemur_bump.tga
    std::string map_bump;
    float map_bump_strength{ 1.f };

    std::string map_roughness;
    std::string map_metalness;
    std::string map_opacity;

    // A material can also have an optical density for its surface. This is also known as index of refraction.
    float ni = 0.0f;

    // Materials can be transparent. This is referred to as being dissolved. Unlike real transparency,
    // the result does not depend upon the thickness of the object.
    // A value of 1.0 for "d" is the default and means fully opaque, as does a value of 0.0 for "Tr".
    // Dissolve works on all illumination models.
    float d = 1.0f;

    int layers = 0;
    float depth = 0.f;

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

    std::string map_dudv;
    std::string map_height;
    std::string map_noise;

    static const int DEFAULT_ID = 0;
private:
    bool m_prepared = false;

    bool m_loaded = false;
};
