#pragma once

#include <string>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <memory>
#include <type_traits>

#include <glm/glm.hpp>

#include "Texture.h"

#include "TextureType.h"
#include "ChannelPart.h"

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
    selection
};

enum class MaterialProgramType : std::underlying_type_t<std::byte> {
    shader,
    shadow,
    pre_depth,
    selection,
    object_id
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
map_Kd textures/texture_cube_512.png
*/

struct Material final
{
public:
    struct BoundTexture {
        Texture* m_texture{ nullptr };
        bool m_channelPart : 1 { false };
        bool m_channelTexture : 1 { false };
    };

public:
    Material();
    Material(Material& o);
    Material(const Material& o);
    Material(Material&& o);
    ~Material();

    Material& operator=(const Material& o);
    Material& operator=(Material&& o);

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

    std::string getTexturePath(
        std::string_view textureName);

    void addTexPath(TextureType type, const std::string& path) noexcept
    {
        if (path.empty()) return;
        //m_texturePaths.insert({ type, path });
        m_texturePaths[type] = path;
    }

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

    const std::unordered_map<TextureType, std::string>& getTexturePaths() const noexcept
    {
        return m_texturePaths;
    }

    std::unordered_map<TextureType, std::string>& modifyTexturePaths() noexcept
    {
        return m_texturePaths;
    }

    const std::string& getTexPath(TextureType type) const noexcept
    {
        const auto& it = m_texturePaths.find(type);
        return it != m_texturePaths.end() ? it->second : "";
    }

    ki::program_id getProgram(MaterialProgramType type) noexcept
    {
        const auto& it = m_programs.find(type);
        return it != m_programs.end() ? it->second : (ki::program_id)0;
    }

private:
    void loadTexture(
        TextureType type,
        bool gammaCorrect,
        bool flipY,
        bool usePlaceholder);

    void loadChannelTexture(
        TextureType channelType,
        std::string_view name,
        const std::vector<ChannelPart>& parts,
        const glm::vec4& defaults);

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

    //// The ambient color of the material is declared using Ka.
    //// HACK KI a bit ambient even if forgotten
    //// => otherwise total blackness shall happen in few cases
    //glm::vec3 ka { 0.01f, 0.01f, 0.01f };

    // Similarly, the diffuse color is declared using Kd.
    glm::vec4 kd { 1.f, 1.f, 1.f, 1.f };
    //std::string map_kd;

    // The specular color is declared using Ks, and weighted using the specular exponent Ns.
    //glm::vec3 ks { 0.f };
    //std::string map_ks;

    // Ke/map_Ke     # emissive
    glm::vec4 ke { 0.f };
    //std::string map_ke;

    // some implementations use 'map_bump' instead of 'bump' below
    // bump map(which by default uses luminance channel of the image)
    // bump lemur_bump.tga
    //std::string map_bump;

    // channel: metalness, roughness, displacement, ambient-occlusion
    glm::vec4 metal{ 0.f, 1.f, 0.f, 1.f };

    //std::string map_roughness;
    //std::string map_metalness;
    //std::string map_occlusion;
    //std::string map_displacement;
    //std::string map_opacity;

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

    //std::string map_dudv;
    //std::string map_noise;

    //static const ki::material_id DEFAULT_ID = 0;

    std::string m_name;
    std::string m_path;

    uint8_t spriteCount = 1;
    uint8_t spritesX = 1;
    //uint8_t spritesY = 1;

    bool alpha : 1 {false};
    bool blend : 1 {false};

    bool renderBack : 1 {false};
    bool lineMode : 1 {false};

    bool gbuffer : 1 {false};
    bool inmutable : 1 {false};

    std::string m_geometryType;

    std::vector<ChannelPart> map_channelParts;

    std::unordered_map<MaterialProgramType, std::string> m_programNames{};
    std::map<std::string, std::string> m_programDefinitions{};

    std::unordered_map<MaterialProgramType, ki::program_id> m_programs{};

    ki::sid m_updaterId;

    MaterialUpdater* m_updater{ nullptr };

private:
    std::unordered_map<TextureType, BoundTexture> m_boundTextures{};
    std::unordered_map<TextureType, std::string> m_texturePaths{};

    ki::material_id m_id;

    bool m_prepared : 1 {false};
    bool m_loaded : 1 {false};
};
