#include "Material.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <mutex>

#include "ImageTexture.h"

#include "scene/RenderContext.h"

namespace {
    constexpr int DIFFUSE_IDX = 0;
    constexpr int EMISSION_IDX = 1;
    constexpr int SPECULAR_IDX = 2;
    constexpr int NORMAL_MAP_IDX = 3;
    constexpr int DUDV_MAP_IDX = 4;

    int idBase = 0;

    std::mutex type_id_lock;

    int nextID()
    {
        std::lock_guard<std::mutex> lock(type_id_lock);
        return ++idBase;
    }
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

Material Material::createDefaultMaterial() {
    Material mat;
    mat.m_name = "<default>";
    mat.ns = 100.f;
    mat.ks = glm::vec4(0.9f, 0.9f, 0.0f, 1.f);
    mat.ka = glm::vec4(0.3f, 0.3f, 0.0f, 1.f);
    mat.kd = glm::vec4(0.8f, 0.8f, 0.0f, 1.f);
    return mat;
}

Material Material::createMaterial(BasicMaterial type)
{
    switch (type) {
    case BasicMaterial::gold: return createGoldMaterial();
    case BasicMaterial::silver: return createSilverMaterial();
    case BasicMaterial::bronze: return createBronzeMaterial();
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

Material::Material()
    : m_objectID(nextID())
{
}

//Material& Meterial::operator=(const Material& m)
//    : objectID(m.objectID)
//{
//    return *this;
//}

Material::~Material()
{
    KI_INFO_SB("MATERIAL: " << m_name << " delete");
}

void Material::loadTextures(const Assets& assets)
{
    if (m_loaded) return;
    m_loaded = true;

    auto baseDir = resolveBaseDir(assets);

    loadTexture(assets, DIFFUSE_IDX, baseDir, map_kd);
    loadTexture(assets, EMISSION_IDX, baseDir, map_ke);
    loadTexture(assets, SPECULAR_IDX, baseDir, map_ks);
    loadTexture(assets, NORMAL_MAP_IDX, baseDir, map_bump);
    loadTexture(assets, DUDV_MAP_IDX, baseDir, map_dudv);
}

std::string Material::resolveBaseDir(const Assets& assets)
{
    std::string baseDir;
    switch (m_type) {
    case MaterialType::model:
        return assets.modelsDir;
    case MaterialType::texture:
        return assets.texturesDir;
    case MaterialType::sprite:
        return assets.spritesDir;
    }
    return assets.modelsDir;
}

void Material::loadTexture(
    const Assets& assets,
    int idx,
    const std::string& baseDir,
    const std::string& textureName)
{
    if (textureName.empty()) return;

    std::string texturePath;
    {
        std::filesystem::path fp;
        fp /= baseDir;
        fp /= m_path;
        fp /= textureName;
        texturePath = fp.string();
    }

    KI_INFO_SB("TEXTURE: " << texturePath);

    std::string placeholderPath = assets.placeholderTexture;
    auto texture = ImageTexture::getTexture(
        assets.placeholderTextureAlways ? placeholderPath : texturePath,
        textureSpec);
    if (!texture->isValid()) {
        texture = ImageTexture::getTexture(placeholderPath, textureSpec);
    }
    assert(texture->isValid());
    if (texture->isValid()) {
        m_textures[idx].texture = texture;
    }
}

int Material::getActiveTextureCount() const
{
    int texCount = 0;
    for (auto& tex : m_textures) {
        if (!tex.texture) continue;
        texCount++;
    }
    return texCount;
}

bool Material::hasNormalTex() const
{
    const auto& tex = m_textures[NORMAL_MAP_IDX];
    return tex.texture != nullptr;
}

void Material::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    for (auto& tex : m_textures) {
        if (!tex.texture) continue;
        tex.texture->prepare(assets);
    }
}

void Material::bindArray(
    const RenderContext& ctx,
    Shader* shader)
{
    for (auto& tex : m_textures) {
        if (!tex.texture) continue;
        ASSERT_TEX_INDEX(tex.m_texIndex);
        tex.bind(ctx);
    }
}

const MaterialUBO Material::toUBO() const
{
    for (auto& tex : m_textures) {
        if (!tex.texture) continue;
        ASSERT_TEX_INDEX(tex.m_texIndex);
    }

    return {
        ka,
        kd,
        glm::vec4(0),
        ks,
        ns,

        m_textures[DIFFUSE_IDX].m_texIndex,
        m_textures[EMISSION_IDX].m_texIndex,
        m_textures[SPECULAR_IDX].m_texIndex,
        m_textures[NORMAL_MAP_IDX].m_texIndex,
        m_textures[DUDV_MAP_IDX].m_texIndex,

        pattern,

        reflection,
        refraction,
        getRefractionRatio(),

        fogRatio,
        tiling,
    };
}

const MaterialSSBO Material::toSSBO() const
{
    for (auto& tex : m_textures) {
        if (!tex.texture) continue;
        ASSERT_TEX_INDEX(tex.m_texIndex);
    }

    return {
        ka,
        kd,
        glm::vec4(0),
        ks,
        ns,

        m_textures[DIFFUSE_IDX].m_texIndex,
        m_textures[EMISSION_IDX].m_texIndex,
        m_textures[SPECULAR_IDX].m_texIndex,
        m_textures[NORMAL_MAP_IDX].m_texIndex,
        m_textures[DUDV_MAP_IDX].m_texIndex,

        pattern,

        reflection,
        refraction,
        getRefractionRatio(),

        fogRatio,
        tiling,
    };
}
