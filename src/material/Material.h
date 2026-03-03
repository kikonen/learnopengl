#pragma once

#include <string>
#include <vector>
#include <array>
#include <map>
#include <memory>
#include <type_traits>
#include <tuple>

#include <glm/glm.hpp>

#include "ki/sid.h"
#include "kigl/kigl.h"

#include "TextureType.h"
#include "TextureSpec.h"

#include "util/UVTransform.h"

//#include "MaterialSSBO.h"

class Program;
class MaterialUpdater;
struct MaterialSSBO;

enum class BasicMaterial : std::underlying_type_t<std::byte> {
    basic,
    black,
    white,
    red,
    green,
    blue,
    yellow,
    gold,
    silver,
    bronze,
    highlight,
    selection,
    wireframe
};

enum class MaterialProgramType : std::underlying_type_t<std::byte> {
    shader,
    oit,
    shadow,
    pre_depth,
    selection,
    object_id,
    normal,
};

struct TextureInfo {
    std::string path;
    bool compressed;
};


class Texture;
class InlineTexture;
class ImageTexture;


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
map_Kd textures/texture_cube_512.png
*/

struct Material final
{
public:
    struct BoundTexture {
        std::shared_ptr<Texture> m_texture;
    };

public:
    Material();
    Material(Material& o);
    Material(const Material& o);
    Material(Material&& o) noexcept;
    ~Material();

    Material& operator=(const Material& o);
    Material& operator=(Material&& o) noexcept;

    std::string str() const noexcept;

    // assign data from other material, but keep local ID
    // NOTE KI *MUST* keep original materialId
    // => it's referred by verteces
    void assign(const Material& o);

    void loadTextures();

    // NOTE KI register can done only once, after which material
    // is effectively *inmmutable*
    //
    // @return m_registeredIndex
    ki::material_index registerMaterial();

    void prepare();

    const MaterialSSBO toSSBO() const;

    unsigned int getFlags() const;

    const float getRefractionRatio() const noexcept{
        return refractionRatio;
    }

    static Material createMaterial(BasicMaterial type);

    static Material* find(
        std::string_view name,
        std::vector<Material>& materials);

    //static Material* findID(
    //    const ki::material_id id,
    //    std::vector<Material>& materials);

    //static const Material* findID(
    //    const ki::material_id id,
    //    const std::vector<Material>& materials);

    std::string resolveTexturePath(
        std::string_view textureName,
        bool compressed);

    // @param compressed use compressed if possible
    void addTexture(
        TextureType type,
        const std::string& path,
        bool compressed) noexcept;

    void addinlineTexture(
        TextureType type,
        const std::shared_ptr<InlineTexture>& texture) noexcept;

    bool hasRegisteredTex(TextureType type) const noexcept
    {
        return m_texturePaths.find(type) != m_texturePaths.end();
    }

    bool hasBoundTex(TextureType type) const noexcept
    {
        return m_boundTextures.find(type) != m_boundTextures.end();
    }

    const BoundTexture* getBoundTex(TextureType type) const noexcept
    {
        const auto& it = m_boundTextures.find(type);
        return it != m_boundTextures.end() ? &it->second : nullptr;
    }

    GLuint64 getTexHandle(TextureType type, GLuint64 defaultValue) const noexcept;

    const std::map<TextureType, TextureInfo>& getTextures() const noexcept
    {
        return m_texturePaths;
    }

    ki::program_id getProgram(MaterialProgramType type) noexcept
    {
        const auto& it = m_programs.find(type);
        return it != m_programs.end() ? it->second : (ki::program_id)0;
    }

private:
    void loadTexture(
        TextureType type,
        bool grayScale,
        bool gammaCorrect,
        bool flipY,
        bool usePlaceholder);

public:
    mutable ki::material_index m_registeredIndex{ -1 };

    TextureSpec textureSpec;

    int pattern = -1;
    float reflection = 0.f;
    float refraction = 0.f;
    float refractionRatio{ 0 };

    float tilingX = 1.0f;
    float tilingY = 1.0f;

    float map_bump_strength{ 1.f };

    //// The specular color is declared using Ks, and weighted using the specular exponent Ns.
    //// ranges between 0 and 1000
    //float ns = 0.0f;

    // Similarly, the diffuse color is declared using Kd.
    glm::vec4 kd { 1.f, 1.f, 1.f, 1.f };
    glm::vec4 ke { 0.f };

    // GLB/GLTF order + KHR_materials_specular
    // MRAS: [metalness, roughness, ambient-occlusion, specular]
    // - metalness (Red):   0 = dielectric, 1 = metal
    // - roughness (Green): 0 = smooth/shiny, 1 = rough/matte
    // - occlusion (Blue):  0 = fully occluded, 1 = no occlusion
    // - specular  (Alpha): 0 = no reflection, 1 = strong reflection
    glm::vec4 mras{ 0.f, 1.f, 1.f, 0.f };

    float m_occlusionFactor{ 1.f };
    float m_metalnessFactor{ 1.f };
    float m_roughnessFactor{ 1.f };

    bool m_invertOcclusion : 1{ false };
    bool m_invertMetalness : 1{ false };
    bool m_invertRoughness : 1{ false };

    //// A material can also have an optical density for its surface. This is also known as index of refraction.
    //float ni = 0.0f;

    //// Materials can be transparent. This is referred to as being dissolved. Unlike real transparency,
    //// the result does not depend upon the thickness of the object.
    //// A value of 1.0 for "d" is the default and means fully opaque, as does a value of 0.0 for "Tr".
        //// Dissolve works on all illumination models.
    //float d = 1.0f;

    int layers = 0;
    float layersDepth = 0.f;
    float parallaxDepth = 0.f;

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
    //int illum = 0;

    std::string m_name;

    uint8_t spriteCount = 1;
    uint8_t spritesX = 1;
    //uint8_t spritesY = 1;

    bool alpha : 1 {false};
    bool blend : 1 {false};

    bool renderBack : 1 {false};
    bool lineMode : 1 {false};
    bool reverseFrontFace : 1 {false};
    bool noDepth : 1 {false};

    bool gbuffer : 1 {false};
    bool inmutable : 1 {false};

    std::string m_geometryType;
    std::string m_baseDir;
    std::string m_modelDir;

    bool m_defaultPrograms{ false };
    std::map<MaterialProgramType, std::string> m_programNames{};

    std::map<std::string, std::string> m_sharedDefinitions{};
    std::map<std::string, std::string> m_programDefinitions{};
    std::map<std::string, std::string> m_oitDefinitions{};
    std::map<std::string, std::string> m_shadowDefinitions{};
    std::map<std::string, std::string> m_selectionDefinitions{};
    std::map<std::string, std::string> m_idDefinitions{};
    std::map<std::string, std::string> m_normalDefinitions{};

    std::map<MaterialProgramType, ki::program_id> m_programs{};

    ki::StringID m_updaterId;

    MaterialUpdater* m_updater{ nullptr };

private:
    std::map<TextureType, BoundTexture> m_boundTextures{};
    std::map<TextureType, TextureInfo> m_texturePaths{};
    std::map<TextureType, std::shared_ptr<InlineTexture>> m_inlineTextures{};

    std::map<TextureType, util::UVTransform> m_textureTransforms;

    ki::material_id m_id;

    bool m_prepared : 1 {false};
    bool m_loaded : 1 {false};
};
