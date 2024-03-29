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
        [&name](Material& m) { return m.m_name == name && !m.m_default; });
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

void Material::loadTextures()
{
    if (m_loaded) return;
    m_loaded = true;

    loadTexture(MATERIAL_DIFFUSE_IDX, map_kd, true, true);
    loadTexture(MATERIAL_EMISSION_IDX, map_ke, true, false);
    loadTexture(MATERIAL_SPECULAR_IDX, map_ks, false, false);
    loadTexture(MATERIAL_NORMAL_MAP_IDX, map_bump, false, false);
    loadTexture(MATERIAL_DUDV_MAP_IDX, map_dudv, false, false);
    loadTexture(MATERIAL_NOISE_MAP_IDX, map_noise, false, false);
    loadTexture(MATERIAL_METALNESS_MAP_IDX, map_metalness, false, false);
    loadTexture(MATERIAL_ROUGHNESS_MAP_IDX, map_roughness, false, false);
    loadTexture(MATERIAL_DISPLACEMENT_MAP_IDX, map_displacement, false, false);
    loadTexture(MATERIAL_OCCLUSION_MAP_IDX, map_occlusion, false, false);
    loadTexture(MATERIAL_OPACITY_MAP_IDX, map_opacity, false, false);

    loadChannelTexture(
        MATERIAL_METAL_CHANNEL_MAP_IDX,
        fmt::format("material_{}_metal", m_registeredIndex),
        {
            MATERIAL_METALNESS_MAP_IDX,
            MATERIAL_ROUGHNESS_MAP_IDX,
            MATERIAL_DISPLACEMENT_MAP_IDX,
            MATERIAL_OCCLUSION_MAP_IDX
        },
        metal);
}

std::string Material::resolveBaseDir()
{
    const auto& assets = Assets::get();

    std::string baseDir;
    switch (m_type) {
    case MaterialType::asset:
        return assets.assetsDir;
    case MaterialType::model:
        return assets.modelsDir;
    case MaterialType::texture:
        return assets.texturesDir;
    case MaterialType::sprite:
        return assets.spritesDir;
    }
    return assets.assetsDir;
}

void Material::loadTexture(
    int idx,
    std::string_view textureName,
    bool gammaCorrect,
    bool usePlaceholder)
{
    if (textureName.empty()) return;

    const auto& assets = Assets::get();

    std::string texturePath = getTexturePath(textureName);

    KI_INFO(fmt::format("MATERIAL: ID={}, name={}, texture={}", m_id, m_name, texturePath));

    const std::string& placeholderPath = assets.placeholderTexture;

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
        future = ImageTexture::getTexture("tex-placeholder", placeholderPath, gammaCorrect, textureSpec);
        future.wait();
        if (future.valid()) {
            texture = future.get();
        }
    }

    if (texture && texture->isValid()) {
        m_textures[idx].m_texture = texture;
    }
}

void Material::loadChannelTexture(
    int idx,
    std::string_view name,
    const std::vector<int>& textureIndeces,
    const glm::vec4& defaults)
{
    const auto& assets = Assets::get();

    std::vector<ImageTexture*> sourceTextures;

    int validCount = 0;
    for (auto sourceIndex : textureIndeces) {
        auto& bound = m_textures[sourceIndex];
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
        m_textures[idx].m_texture = texture;
        m_textures[idx].m_channelTexture = true;
    }
}

std::string Material::getTexturePath(
    std::string_view textureName)
{
    if (textureName.empty()) return {};

    std::string texturePath;
    {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        texturePath = util::joinPathExt(
            resolveBaseDir(),
            m_path,
            textureName, "");
    }
    return texturePath;
}

bool Material::hasTex(int index) const
{
    const auto& tex = m_textures[index];
    return tex.m_texture != nullptr;
}

void Material::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    for (auto& tex : m_textures) {
        if (!tex.m_texture) continue;
        if (tex.m_channelPart) continue;

        tex.m_texture->prepare();
        //tex.m_texIndex = tex.m_texture->m_texIndex;
        tex.m_handle = tex.m_texture->m_handle;
    }
}

const MaterialSSBO Material::toSSBO() const
{
    for (auto& tex : m_textures) {
        if (!tex.m_texture) continue;
        if (tex.m_channelPart) continue;
        //ASSERT_TEX_INDEX(tex.m_texIndex);
    }

    const auto& whitePx = ColorTexture::getWhiteRGBA().m_handle;

    return {
        kd,
        ke,

        m_textures[MATERIAL_METAL_CHANNEL_MAP_IDX].m_handle ? WHITE_RGBA : metal,

        m_textures[MATERIAL_DIFFUSE_IDX].m_handle ? m_textures[MATERIAL_DIFFUSE_IDX].m_handle : whitePx,
        m_textures[MATERIAL_EMISSION_IDX].m_handle ? m_textures[MATERIAL_EMISSION_IDX].m_handle : whitePx,

        m_textures[MATERIAL_NORMAL_MAP_IDX].m_handle,
        m_textures[MATERIAL_DUDV_MAP_IDX].m_handle,
        m_textures[MATERIAL_NOISE_MAP_IDX].m_handle,

        m_textures[MATERIAL_OPACITY_MAP_IDX].m_handle ? m_textures[MATERIAL_OPACITY_MAP_IDX].m_handle : whitePx,
        m_textures[MATERIAL_METAL_CHANNEL_MAP_IDX].m_handle ? m_textures[MATERIAL_METAL_CHANNEL_MAP_IDX].m_handle : whitePx,

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
