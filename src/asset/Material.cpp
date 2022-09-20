#include "Material.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <mutex>

#include "ImageTexture.h"

namespace {
    constexpr int DIFFUSE_IDX = 0;
    constexpr int EMISSION_IDX = 1;
    constexpr int SPECULAR_IDX = 2;
    constexpr int NORMAL_MAP_IDX = 3;
    constexpr int DUDV_MAP_IDX = 4;

    int typeIDbase = 0;

    std::mutex type_id_lock;

    int nextID()
    {
        std::lock_guard<std::mutex> lock(type_id_lock);
        return ++typeIDbase;
    }
}

std::shared_ptr<Material> createGoldMaterial() {
    std::shared_ptr<Material> mat = std::make_shared<Material>();
    mat->name = "<gold>";
    mat->ns = 51.2f;
    mat->ks = glm::vec4(0.6283f, 0.5559f, 0.3661f, 1.f);
    mat->ka = glm::vec4(0.2473f, 0.1995f, 0.0745f, 1.f);
    mat->kd = glm::vec4(0.7516f, 0.6065f, 0.2265f, 1.f);
    return mat;
}

std::shared_ptr<Material> createSilverMaterial() {
    std::shared_ptr<Material> mat = std::make_shared<Material>();
    mat->name = "<silver>";
    mat->ns = 51.2f;
    mat->ks = glm::vec4(0.5083f, 0.5083f, 0.5083f, 1.f);
    mat->ka = glm::vec4(0.1923f, 0.1923f, 0.1923f, 1.f);
    mat->kd = glm::vec4(0.5075f, 0.5075f, 0.5075f, 1.f);
    return mat;
}

std::shared_ptr<Material> createBronzeMaterial() {
    std::shared_ptr<Material> mat = std::make_shared<Material>();
    mat->name = "<bronze>";
    mat->ns = 25.6f;
    mat->ks = glm::vec4(0.3936f, 0.2719f, 0.1667f, 1.f);
    mat->ka = glm::vec4(0.2125f, 0.1275f, 0.0540f, 1.f);
    mat->kd = glm::vec4(0.7140f, 0.4284f, 0.1814f, 1.f);
    return mat;
}

std::shared_ptr<Material> Material::createDefaultMaterial() {
    std::shared_ptr<Material> mat = std::make_shared<Material>();
    mat->name = "<default>";
    mat->ns = 100.f;
    mat->ks = glm::vec4(0.9f, 0.9f, 0.0f, 1.f);
    mat->ka = glm::vec4(0.3f, 0.3f, 0.0f, 1.f);
    mat->kd = glm::vec4(0.8f, 0.8f, 0.0f, 1.f);
    return mat;
}

std::shared_ptr<Material> Material::createMaterial(BasicMaterial type)
{
    switch (type) {
    case BasicMaterial::gold: return createGoldMaterial();
    case BasicMaterial::silver: return createSilverMaterial();
    case BasicMaterial::bronze: return createBronzeMaterial();
    }

    return createDefaultMaterial();
}

std::shared_ptr<Material> Material::find(
    const std::string& name,
    std::vector<std::shared_ptr<Material>>& materials)
{
    const auto& e = std::find_if(
        materials.begin(),
        materials.end(),
        [&name](std::shared_ptr<Material>& m) { return m->name == name && !m->isDefault; });
    return e != materials.end() ? *e : nullptr;
}

std::shared_ptr<Material> Material::findID(
    const int objectID,
    std::vector<std::shared_ptr<Material>>& materials)
{
    const auto& e = std::find_if(
        materials.begin(),
        materials.end(),
        [objectID](std::shared_ptr<Material>& m) { return m->objectID == objectID; });
    return e != materials.end() ? *e : nullptr;
}

Material::Material()
    : objectID(nextID())
{
}

Material::~Material()
{
    KI_INFO_SB("MATERIAL: " << name << " delete");
    materialIndex = -1;
}

void Material::loadTextures(const Assets& assets)
{
    if (loaded) return;
    loaded = true;

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
    switch (type) {
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
        fp /= path;
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
        textures[idx].texture = texture;
    }
}

void Material::prepare(const Assets& assets)
{
    // MOTE KI this loop is for "non ModelMesh" case (like sprite)
    unsigned int unitIndex = 0;
    for (auto& tex : textures) {
        if (!tex.texture) continue;
        tex.unitIndex = unitIndex++;
        tex.texture->prepare(assets);
    }
}

void Material::bindArray(Shader* shader, int index, bool bindTextureIDs)
{
    for (auto& tex : textures) {
        if (!tex.texture) continue;
        assert(tex.unitIndex >= 0);
        shader->textures[tex.unitIndex].set(tex.unitIndex);
        if (bindTextureIDs) {
            tex.bind();
        }
    }
}

MaterialUBO Material::toUBO()
{
    return {
        ka,
        kd,
        glm::vec4(0),
        ks,
        ns,

        textures[DIFFUSE_IDX].unitIndex,
        textures[EMISSION_IDX].unitIndex,
        textures[SPECULAR_IDX].unitIndex,
        textures[NORMAL_MAP_IDX].unitIndex,
        textures[DUDV_MAP_IDX].unitIndex,

        pattern,

        reflection,
        refraction,
        getRefractionRatio(),

        fogRatio,
        tiling,
    };
}
