#include "Material.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <mutex>

#include "fmt/format.h"

#include "util/Util.h"

#include "asset/Shader.h"

#include "asset/ImageTexture.h"
#include "asset/ChannelTexture.h"

#include "asset/MaterialSSBO.h"
#include "asset/TextureUBO.h"


namespace {
    int idBase = 0;

    std::mutex type_id_lock{};

    int nextID()
    {
        std::lock_guard<std::mutex> lock(type_id_lock);
        return ++idBase;
    }

    float calculateAmbient(glm::vec3 ambient) {
        return (ambient.x + ambient.y + ambient.z) / 3.f;
    }

    Material createDefaultMaterial() {
        Material mat;
        mat.m_name = "<default>";
        mat.ns = 100.f;
        mat.ks = glm::vec4(0.9f, 0.9f, 0.0f, 1.f);
        mat.ka = glm::vec4(0.3f, 0.3f, 0.0f, 1.f);
        mat.kd = glm::vec4(0.8f, 0.8f, 0.0f, 1.f);
        return mat;
    }

    Material createBasicMaterial() {
        Material mat;
        mat.m_name = "<basic>";
        mat.ns = 100.f;
        mat.ks = glm::vec4(0.9f, 0.9f, 0.0f, 1.f);
        mat.ka = glm::vec4(0.3f, 0.3f, 0.0f, 1.f);
        mat.kd = glm::vec4(0.8f, 0.8f, 0.0f, 1.f);
        return mat;
    }

    Material createGoldMaterial() {
        Material mat;
        mat.m_name = "<gold>";
        mat.ns = 51.2f;
        mat.ks = glm::vec4(0.6283f, 0.5559f, 0.3661f, 1.f);
        mat.ka = glm::vec4(0.2473f, 0.1995f, 0.0745f, 1.f);
        mat.kd = glm::vec4(0.7516f, 0.6065f, 0.2265f, 1.f);
        return mat;
    }

    Material createSilverMaterial() {
        Material mat;
        mat.m_name = "<silver>";
        mat.ns = 51.2f;
        mat.ks = glm::vec4(0.5083f, 0.5083f, 0.5083f, 1.f);
        mat.ka = glm::vec4(0.1923f, 0.1923f, 0.1923f, 1.f);
        mat.kd = glm::vec4(0.5075f, 0.5075f, 0.5075f, 1.f);
        return mat;
    }

    Material createBronzeMaterial() {
        Material mat;
        mat.m_name = "<bronze>";
        mat.ns = 25.6f;
        mat.ks = glm::vec4(0.3936f, 0.2719f, 0.1667f, 1.f);
        mat.ka = glm::vec4(0.2125f, 0.1275f, 0.0540f, 1.f);
        mat.kd = glm::vec4(0.7140f, 0.4284f, 0.1814f, 1.f);
        return mat;
    }

    Material createHighlightMaterial() {
        Material mat;
        mat.m_name = "<highlight>";
        mat.ns = 100.f;
        mat.ks = glm::vec4(0.0f, 0.0f, 0.8f, 1.f);
        mat.ka = glm::vec4(0.0f, 0.0f, 0.8f, 1.f);
        mat.kd = glm::vec4(0.0f, 0.0f, 0.8f, 1.f);
        return mat;
    }

    Material createSelectionMaterial() {
        Material mat;
        mat.m_name = "<selection>";
        mat.ns = 100.f;
        mat.ks = glm::vec4(0.8f, 0.0f, 0.0f, 1.f);
        mat.ka = glm::vec4(0.8f, 0.0f, 0.0f, 1.f);
        mat.kd = glm::vec4(0.8f, 0.0f, 0.0f, 1.f);
        return mat;
    }
}

Material Material::createMaterial(BasicMaterial type)
{
    switch (type) {
    case BasicMaterial::basic: return createBasicMaterial();
    case BasicMaterial::gold: return createGoldMaterial();
    case BasicMaterial::silver: return createSilverMaterial();
    case BasicMaterial::bronze: return createBronzeMaterial();
    case BasicMaterial::highlight: return createHighlightMaterial();
    case BasicMaterial::selection: return createSelectionMaterial();
    }

    return createDefaultMaterial();
}

Material* Material::find(
    const std::string& name,
    std::vector<Material>& materials)
{
    const auto& it = std::find_if(
        materials.begin(),
        materials.end(),
        [&name](Material& m) { return m.m_name == name && !m.m_default; });
    return it != materials.end() ? &(*it) : nullptr;
}

Material* Material::findID(
    const int objectID,
    std::vector<Material>& materials)
{
    const auto& it = std::find_if(
        materials.begin(),
        materials.end(),
        [objectID](Material& m) { return m.m_objectID == objectID; });
    return it != materials.end() ? &(*it) : nullptr;
}

const Material* Material::findID(
    const int objectID,
    const std::vector<Material>& materials)
{
    const auto& it = std::find_if(
        materials.begin(),
        materials.end(),
        [objectID](const Material& m) { return m.m_objectID == objectID; });
    return it != materials.end() ? &(*it) : nullptr;
}

Material::Material()
    : m_objectID(nextID())
{
}

Material::~Material()
{
    KI_INFO(fmt::format(
        "MATERIAL: delete - ID={}, name={}, index={}",
        m_objectID, m_name, m_registeredIndex));
}

void Material::loadTextures(const Assets& assets)
{
    if (m_loaded) return;
    m_loaded = true;

    loadTexture(assets, MATERIAL_DIFFUSE_IDX, map_kd, true, true);
    loadTexture(assets, MATERIAL_EMISSION_IDX, map_ke, true, false);
    loadTexture(assets, MATERIAL_SPECULAR_IDX, map_ks, false, false);
    loadTexture(assets, MATERIAL_NORMAL_MAP_IDX, map_bump, false, false);
    loadTexture(assets, MATERIAL_DUDV_MAP_IDX, map_dudv, false, false);
    loadTexture(assets, MATERIAL_HEIGHT_MAP_IDX, map_height, false, false);
    loadTexture(assets, MATERIAL_NOISE_MAP_IDX, map_noise, false, false);
    loadTexture(assets, MATERIAL_METALNESS_MAP_IDX, map_metalness, false, false);
    loadTexture(assets, MATERIAL_ROUGHNESS_MAP_IDX, map_roughness, false, false);
    loadTexture(assets, MATERIAL_DISPLACEMENT_MAP_IDX, map_displacement, false, false);
    loadTexture(assets, MATERIAL_OCCLUSION_MAP_IDX, map_occlusion, false, false);
    loadTexture(assets, MATERIAL_OPACITY_MAP_IDX, map_opacity, false, false);

    loadChannelTexture(
        assets,
        MATERIAL_METAL_CHANNEL_MAP_IDX,
        "metal",
        {
            MATERIAL_METALNESS_MAP_IDX,
            MATERIAL_ROUGHNESS_MAP_IDX,
            MATERIAL_DISPLACEMENT_MAP_IDX,
            MATERIAL_OCCLUSION_MAP_IDX
        });
}

std::string Material::resolveBaseDir(const Assets& assets)
{
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
    const Assets& assets,
    int idx,
    const std::string& textureName,
    bool gammaCorrect,
    bool usePlaceholder)
{
    if (textureName.empty()) return;

    std::string texturePath = getTexturePath(assets, textureName);

    KI_INFO(fmt::format("MATERIAL: ID={}, name={}, texture={}", m_objectID, m_name, texturePath));

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
    const Assets& assets,
    int idx,
    const std::string& name,
    std::vector<int> textureIndeces)
{
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

    KI_INFO(fmt::format("MATERIAL: ID={}, name={}, texture={}", m_objectID, m_name, name));

    const std::string& placeholderPath = assets.placeholderTexture;

    auto future = ChannelTexture::getTexture(
        name,
        sourceTextures,
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

const std::string Material::getTexturePath(
    const Assets& assets,
    const std::string& textureName)
{
    if (textureName.empty()) return {};

    std::string texturePath;
    {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        texturePath = util::joinPath(
            resolveBaseDir(assets),
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

void Material::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    for (auto& tex : m_textures) {
        if (!tex.m_texture) continue;
        if (tex.m_channelPart) continue;

        tex.m_texture->prepare(assets);
        tex.m_texIndex = tex.m_texture->m_texIndex;
        tex.m_handle = tex.m_texture->m_handle;
    }
}

const MaterialSSBO Material::toSSBO() const
{
    for (auto& tex : m_textures) {
        if (!tex.m_texture) continue;
        if (tex.m_channelPart) continue;
        ASSERT_TEX_INDEX(tex.m_texIndex);
    }

    return {
        kd,
        ke,

        glm::vec4{ ks, ns },
        glm::vec4{ 0.f },

        m_textures[MATERIAL_DIFFUSE_IDX].m_handle,
        m_textures[MATERIAL_EMISSION_IDX].m_handle,
        m_textures[MATERIAL_SPECULAR_IDX].m_handle,
        m_textures[MATERIAL_NORMAL_MAP_IDX].m_handle,

        m_textures[MATERIAL_DUDV_MAP_IDX].m_handle,
        m_textures[MATERIAL_HEIGHT_MAP_IDX].m_handle,
        m_textures[MATERIAL_NOISE_MAP_IDX].m_handle,
        m_textures[MATERIAL_OPACITY_MAP_IDX].m_handle,

        m_textures[MATERIAL_METAL_CHANNEL_MAP_IDX].m_handle,

        calculateAmbient(ka),
        pattern,

        reflection,
        refraction,
        getRefractionRatio(),

        tilingX,
        tilingY,

        layers,
        layersDepth,
        parallaxDepth,
    };
}
