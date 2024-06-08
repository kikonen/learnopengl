#include "Material.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "fmt/format.h"

#include "pool/IdGenerator.h"

#include "util/Util.h"

#include "asset/Assets.h"
#include "asset/Shader.h"

#include "asset/ImageTexture.h"
#include "asset/ChannelTexture.h"
#include "asset/ColorTexture.h"

#include "asset/MaterialSSBO.h"
#include "asset/TextureUBO.h"


namespace {
    IdGenerator<ki::material_id> ID_GENERATOR;

    const glm::vec4 WHITE_RGBA{ 1.f };
    const glm::vec4 BLACK_RGBA{ 0.f };

    float calculateAmbient(glm::vec3 ambient) {
        return (ambient.x + ambient.y + ambient.z) / 3.f;
    }

    Material createDefaultMaterial() {
        Material mat;
        mat.m_name = "<default>";
        mat.kd = glm::vec4(0.8f, 0.8f, 0.0f, 1.f);
        return mat;
    }

    Material createBasicMaterial() {
        Material mat;
        mat.m_name = "<basic>";
        mat.kd = glm::vec4(0.8f, 0.8f, 0.0f, 1.f);
        return mat;
    }

    Material createWhiteMaterial() {
        Material mat;
        mat.m_name = "<white>";
        mat.kd = { 1.f, 1.f, 1.f, 1.f };
        return mat;
    }

    Material createGoldMaterial() {
        Material mat;
        mat.m_name = "<gold>";
        mat.kd = glm::vec4(0.7516f, 0.6065f, 0.2265f, 1.f);
        return mat;
    }

    Material createSilverMaterial() {
        Material mat;
        mat.m_name = "<silver>";
        mat.kd = glm::vec4(0.5075f, 0.5075f, 0.5075f, 1.f);
        return mat;
    }

    Material createBronzeMaterial() {
        Material mat;
        mat.m_name = "<bronze>";
        mat.kd = glm::vec4(0.7140f, 0.4284f, 0.1814f, 1.f);
        return mat;
    }

    Material createHighlightMaterial() {
        Material mat;
        mat.m_name = "<highlight>";
        mat.kd = glm::vec4(0.0f, 0.0f, 0.8f, 1.f);
        return mat;
    }

    Material createSelectionMaterial() {
        Material mat;
        mat.m_name = "<selection>";
        mat.kd = glm::vec4(0.8f, 0.0f, 0.0f, 1.f);
        return mat;
    }
}

Material Material::createMaterial(BasicMaterial type)
{
    switch (type) {
    case BasicMaterial::basic: return createBasicMaterial();
    case BasicMaterial::white: return createWhiteMaterial();
    case BasicMaterial::gold: return createGoldMaterial();
    case BasicMaterial::silver: return createSilverMaterial();
    case BasicMaterial::bronze: return createBronzeMaterial();
    case BasicMaterial::highlight: return createHighlightMaterial();
    case BasicMaterial::selection: return createSelectionMaterial();
    }

    return createDefaultMaterial();
}

Material* Material::find(
    std::string_view name,
    std::vector<Material>& materials)
{
    const auto& it = std::find_if(
        materials.begin(),
        materials.end(),
        [&name](Material& m) { return m.m_name == name; });
    return it != materials.end() ? &(*it) : nullptr;
}

Material* Material::findID(
    const ki::material_id id,
    std::vector<Material>& materials)
{
    const auto& it = std::find_if(
        materials.begin(),
        materials.end(),
        [id](Material& m) { return m.m_id == id; });
    return it != materials.end() ? &(*it) : nullptr;
}

const Material* Material::findID(
    const ki::material_id id,
    const std::vector<Material>& materials)
{
    const auto& it = std::find_if(
        materials.begin(),
        materials.end(),
        [id](const Material& m) { return m.m_id == id; });
    return it != materials.end() ? &(*it) : nullptr;
}

Material::Material()
    : m_id(ID_GENERATOR.nextId())
{
}

Material::~Material()
{
    //KI_INFO(fmt::format(
    //    "MATERIAL: delete - ID={}, name={}, index={}",
    //    m_id, m_name, m_registeredIndex));
}

void Material::assign(const Material& o)
{
    auto oldId = m_id;
    *this = o;
    m_id = oldId;
}

void Material::loadTextures()
{
    if (m_loaded) return;
    m_loaded = true;

    const auto& assets = Assets::get();
    auto compressed = assets.compressedTexturesEnabled;

    loadTexture(TextureType::diffuse, true, true);
    loadTexture(TextureType::emission, true, false);
    loadTexture(TextureType::specular, false, false);
    loadTexture(TextureType::normal_map, false, false);
    loadTexture(TextureType::dudv_map, false, false);
    loadTexture(TextureType::noise_map, false, false);
    loadTexture(TextureType::metallness_map, false, false);
    loadTexture(TextureType::roughness_map, false, false);
    loadTexture(TextureType::displacement_map, false, false);
    loadTexture(TextureType::occlusion_map, false, false);
    loadTexture(TextureType::opacity_map, false, false);

    loadChannelTexture(
        TextureType::metal_channel_map,
        fmt::format("material_{}_metal", m_registeredIndex),
        {
            TextureType::metallness_map,
            TextureType::roughness_map,
            TextureType::displacement_map,
            TextureType::occlusion_map,
        },
        metal);
}

void Material::loadTexture(
    TextureType type,
    bool gammaCorrect,
    bool usePlaceholder)
{
    const auto& it = m_texturePaths.find(type);
    if (it == m_texturePaths.end()) return;

    const auto& textureName = it->second;

    const auto& assets = Assets::get();

    std::string texturePath = getTexturePath(textureName);

    KI_INFO(fmt::format("MATERIAL: ID={}, name={}, texture={}", m_id, m_name, texturePath));

    const std::string& placeholderPath = util::joinPath(assets.assetsDir, assets.placeholderTexture);

    auto future = ImageTexture::getTexture(
        textureName,
        usePlaceholder && assets.placeholderTextureAlways ? placeholderPath : texturePath,
        gammaCorrect,
        textureSpec);

    future.wait();

    ImageTexture* texture = { nullptr };
    if (future.valid()) {
        texture = future.get();
    }

    if (usePlaceholder && !texture->isValid()) {
        future = ImageTexture::getTexture(
            "tex-placeholder",
            placeholderPath,
            gammaCorrect,
            textureSpec);

        future.wait();
        if (future.valid()) {
            texture = future.get();
        }
    }

    if (texture && texture->isValid()) {
        m_boundTextures.insert({ type, BoundTexture{ texture } });
    }
}

void Material::loadChannelTexture(
    TextureType channelType,
    std::string_view name,
    const std::vector<TextureType>& compoundTypes,
    const glm::vec4& defaults)
{
    const auto& assets = Assets::get();

    std::vector<ImageTexture*> sourceTextures;

    int validCount = 0;
    for (auto sourceTypes : compoundTypes) {
        const auto& it = m_boundTextures.find(sourceTypes);
        if (it == m_boundTextures.end()) continue;
        auto& bound = it->second;
        if (bound.m_texture) {
            sourceTextures.push_back((ImageTexture*)bound.m_texture);
            bound.m_channelPart = true;
            validCount++;
        }
        else {
            sourceTextures.push_back(nullptr);
        }
    }

    KI_INFO(fmt::format("MATERIAL: ID={}, name={}, texture={}, validCount={}", m_id, m_name, name, validCount));

    if (validCount == 0) return;

    const std::string& placeholderPath = assets.placeholderTexture;

    auto future = ChannelTexture::getTexture(
        name,
        sourceTextures,
        defaults,
        false,
        textureSpec);

    future.wait();

    ChannelTexture* texture{ nullptr };

    if (future.valid()) {
        texture = future.get();
    }

    if (texture && texture->isValid()) {
        m_boundTextures.insert({ channelType, {} });
        const auto& it = m_boundTextures.find(channelType);
        auto& bound = it->second;
        bound.m_texture = texture;
        bound.m_channelTexture = true;
    }
}

std::string Material::getTexturePath(
    std::string_view textureName)
{
    if (textureName.empty()) return {};

    std::string texturePath;
    {
        const auto& assets = Assets::get();

        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        texturePath = util::joinPathExt(
            assets.assetsDir,
            m_path,
            textureName, "");
    }
    return texturePath;
}

void Material::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    for (auto& it : m_boundTextures) {
        auto& tex = it.second;
        if (tex.m_channelPart) continue;
        tex.m_texture->prepare();
    }
}

const MaterialSSBO Material::toSSBO() const
{
    const auto& whitePx = ColorTexture::getWhiteRGBA().m_handle;

    return {
        kd,

        hasBoundTex(TextureType::emission) ? WHITE_RGBA : ke,
        hasBoundTex(TextureType::metal_channel_map) ? WHITE_RGBA : metal,

        getTexHandle(TextureType::diffuse, whitePx),
        getTexHandle(TextureType::emission, whitePx),

        getTexHandle(TextureType::normal_map, 0),
        getTexHandle(TextureType::dudv_map, 0),
        getTexHandle(TextureType::noise_map, 0),

        getTexHandle(TextureType::opacity_map, whitePx),
        getTexHandle(TextureType::metal_channel_map, whitePx),

        pattern,

        reflection,
        refraction,
        getRefractionRatio(),

        tilingX,
        tilingY,

        spriteCount,
        spritesX,
        spritesY,

        layers,
        layersDepth,
        parallaxDepth,
    };
}
