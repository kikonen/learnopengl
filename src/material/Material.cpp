#include "Material.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <tuple>

#include "fmt/format.h"

#include "pool/IdGenerator.h"

#include "util/util.h"
#include "util/file.h"
#include "util/thread.h"

#include "asset/Assets.h"

#include "shader/Shader.h"

#include "material/ImageRegistry.h"

#include "ImageTexture.h"
#include "InlineTexture.h"
#include "ColorTexture.h"

#include "MaterialSSBO.h"
#include "MaterialRegistry.h"
#include "MaterialUpdater.h"

namespace {
    IdGenerator<ki::material_id> ID_GENERATOR;

    constexpr unsigned int MATERIAL_INVERT_OCCLUSION = 1;
    constexpr unsigned int MATERIAL_INVERT_METALNESS = 2;
    constexpr unsigned int MATERIAL_INVERT_ROUGHNESS = 4;

    const glm::vec4 WHITE_RGBA{ 1.f };
    const glm::vec4 BLACK_RGBA{ 0.f };

    const std::regex CONTAINS_BUILD = std::regex(".*_build.*");

    //float calculateAmbient(glm::vec3 ambient) {
    //    return (ambient.x + ambient.y + ambient.z) / 3.f;
    //}

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

    Material createRGBMaterial(std::string_view name, const glm::vec4& color) {
        Material mat;
        mat.m_name = name;
        mat.kd = color;
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

    Material createWireframeMaterial() {
        Material mat;
        mat.m_name = "<wireframe>";
        mat.kd = glm::vec4(0.0f, 0.8f, 0.0f, 1.f);
        return mat;
    }

    std::pair<std::string, bool> selectTexturePath(
        std::string_view path,
        bool useCompressed)
    {
        const auto& assets = Assets::get();

        std::filesystem::path filePath;

        bool found = false;

        {
            std::filesystem::path buildPath{ path };
            const auto& stem = buildPath.stem().string();

            if (std::regex_match(stem, CONTAINS_BUILD)) {
                buildPath.replace_filename(fmt::format("{}.{}", stem, "png"));
            }
            else {
                buildPath.replace_filename(fmt::format("{}_build.{}", stem, "png"));
            }

            if (useCompressed && assets.compressedTexturesEnabled) {
                std::filesystem::path ktxPath{ buildPath };
                ktxPath.replace_extension(".ktx");

                const auto fullPath = util::joinPath(
                    assets.assetsBuildDir,
                    ktxPath.string());

                if (util::fileExists(fullPath)) {
                    filePath = fullPath;
                    found = true;
                }
            }

            //const auto re = std::regex(".*scenery_build.png");
            //if (std::regex_match(buildPath.string(), re)) {
            //    int x = 0;
            //}

            if (!found) {
                const auto fullPath = util::joinPath(
                    assets.assetsBuildDir,
                    buildPath.string());

                if (util::fileExists(fullPath)) {
                    filePath = fullPath;
                    found = true;
                }
            }
        }

        if (!found) {
            filePath = util::joinPath(assets.assetsDir, path);
        }

        return { filePath.string(), found };
    }
}

Material Material::createMaterial(BasicMaterial type)
{
    switch (type) {
    case BasicMaterial::basic: return createBasicMaterial();
    case BasicMaterial::black: return createRGBMaterial("<black>", { 0.f, 0.f, 0.f, 1.f });
    case BasicMaterial::white: return createRGBMaterial("<white>", { 1.f, 1.f, 1.f, 1.f });
    case BasicMaterial::red: return createRGBMaterial("<red>", { 1.f, 0.f, 0.f, 1.f });
    case BasicMaterial::green: return createRGBMaterial("<green>", { 0.f, 1.f, 0.f, 1.f });
    case BasicMaterial::blue: return createRGBMaterial("<blue>", { 0.f, 0.f, 1.f, 1.f });
    case BasicMaterial::yellow: return createRGBMaterial("<yellow>", { 1.f, 1.f, 0.f, 1.f });
    case BasicMaterial::gold: return createGoldMaterial();
    case BasicMaterial::silver: return createSilverMaterial();
    case BasicMaterial::bronze: return createBronzeMaterial();
    case BasicMaterial::highlight: return createHighlightMaterial();
    case BasicMaterial::selection: return createSelectionMaterial();
    case BasicMaterial::wireframe: return createWireframeMaterial();
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

//Material* Material::findID(
//    const ki::material_id id,
//    std::vector<Material>& materials)
//{
//    const auto& it = std::find_if(
//        materials.begin(),
//        materials.end(),
//        [id](Material& m) { return m.m_id == id; });
//    return it != materials.end() ? &(*it) : nullptr;
//}
//
//const Material* Material::findID(
//    const ki::material_id id,
//    const std::vector<Material>& materials)
//{
//    const auto& it = std::find_if(
//        materials.begin(),
//        materials.end(),
//        [id](const Material& m) { return m.m_id == id; });
//    return it != materials.end() ? &(*it) : nullptr;
//}

Material::Material()
    : m_id( ID_GENERATOR.nextId() ),
    m_updaterId{ 0 }
{
}

Material::Material(Material& o) = default;
Material::Material(const Material& o) = default;

Material::Material(Material&& o) noexcept = default;
//    : m_registeredIndex{ o.m_registeredIndex },
//    textureSpec{ o.textureSpec },
//    pattern{ o.pattern },
//    reflection{ o.reflection },
//    refraction{ o.refraction },
//    refractionRatio{ o.refractionRatio },
//
//    tilingX{ o.tilingX },
//    tilingY{ o.tilingY },
//
//    map_bump_strength{ o.map_bump_strength },
//
//    kd{ o.kd },
//    ke{ o.ke },
//
//    mras{ o.mras },
//
//    layers{ o.layers },
//    layersDepth{ o.layersDepth },
//    parallaxDepth{ o.parallaxDepth },
//
//    m_name{ o.m_name },
//
//    spriteCount{ o.spriteCount },
//    spritesX{ o.spritesX },
//
//    alpha{ o.alpha },
//    blend{ o.blend },
//
//    renderBack{ o.renderBack },
//    lineMode{ o.lineMode },
//    reverseFrontFace{ o.reverseFrontFace },
//    noDepth{ o.noDepth },
//
//    gbuffer{ o.gbuffer },
//    inmutable{ o.inmutable },
//
//    m_geometryType{ o.m_geometryType },
//    m_baseDir{ o.m_baseDir },
//    m_modelDir{ o.m_modelDir },
//
//    m_defaultPrograms{ o.m_defaultPrograms },
//    m_programNames{ o.m_programNames },
//
//    m_sharedDefinitions{ o.m_sharedDefinitions },
//    m_programDefinitions{ o.m_programDefinitions },
//    m_oitDefinitions{ o.m_oitDefinitions },
//    m_shadowDefinitions{ o.m_shadowDefinitions },
//    m_selectionDefinitions{ o.m_selectionDefinitions },
//    m_idDefinitions{ o.m_idDefinitions },
//    m_normalDefinitions{ o.m_normalDefinitions },
//
//    m_programs{ o.m_programs },
//
//    m_updaterId{ o.m_updaterId },
//
//    m_updater{ o.m_updater },
//
//    m_boundTextures{ o.m_boundTextures },
//    m_texturePaths{ o.m_texturePaths },
//    m_inlineTextures{ o.m_inlineTextures },
//
//    m_id{ o.m_id },
//
//    m_prepared{ o.m_prepared },
//    m_loaded{ o.m_loaded }
//{}

Material::~Material() = default;
//{
//    //KI_INFO(fmt::format(
//    //    "MATERIAL: delete - ID={}, name={}, index={}",
//    //    m_id, m_name, m_registeredIndex));
//}

Material& Material::operator=(const Material& o) = default;
Material& Material::operator=(Material&& o) noexcept = default;

std::string Material::str() const noexcept
{
    return fmt::format(
        "<MATERIAL: name={}, programs={}, definitions={}>",
        m_name,
        m_programNames.size(),
        m_programDefinitions.size());
}


void Material::assign(const Material& o)
{
    auto oldId = m_id;
    auto oldModelDir = m_modelDir;
    *this = o;
    m_id = oldId;
    m_modelDir = oldModelDir;
}

ki::material_index Material::registerMaterial()
{
    return MaterialRegistry::get().registerMaterial(*this);
}

GLuint64 Material::getTexHandle(TextureType type, GLuint64 defaultValue) const noexcept
{
    if (m_updater) {
        auto handle = m_updater->getTexHandle(type);
        if (handle) return handle;
    }
    const auto& it = m_boundTextures.find(type);
    return it != m_boundTextures.end() ? it->second.m_texture->m_handle : defaultValue;
}

void Material::loadTextures()
{
    if (m_loaded) return;
    m_loaded = true;

    const auto& assets = Assets::get();
    auto compressed = assets.compressedTexturesEnabled;

    for (const auto& it : m_texturePaths) {
        const auto type = it.first;
        bool grayScale = false;
        bool gammaCorrect = false;
        bool flipY = true;
        bool usePlaceholder = false;

        if (type == TextureType::diffuse) {
            grayScale = true;
            gammaCorrect = true;
            usePlaceholder = true;
        }
        else if (type == TextureType::emission) {
            grayScale = true;
            gammaCorrect = true;
        }

        loadTexture(type, grayScale, gammaCorrect, flipY, usePlaceholder);
    }

    for (const auto& it : m_inlineTextures) {
        const auto type = it.first;
        const auto& texture = it.second;
        if (texture && texture->isValid()) {
            m_boundTextures.insert({ type, BoundTexture{ texture } });
        }
    }
}

void Material::loadTexture(
    TextureType type,
    bool grayScale,
    bool gammaCorrect,
    bool flipY,
    bool usePlaceholder)
{
    const auto& it = m_texturePaths.find(type);
    if (it == m_texturePaths.end()) return;

    const auto& info = it->second;

    const auto& assets = Assets::get();

    std::string texturePath = resolveTexturePath(info.path, info.compressed);

    KI_INFO(fmt::format("TEX::LOAD: ID={}, name={}, texture={}", m_id, m_name, texturePath));

    const std::string& placeholderPath = util::joinPath(assets.assetsDir, assets.placeholderTexture);

    auto future = ImageRegistry::get().getTexture(
        info.path,
        usePlaceholder && assets.placeholderTextureAlways ? placeholderPath : texturePath,
        false,
        grayScale,
        gammaCorrect,
        flipY,
        type,
        textureSpec);

    future.wait();

    std::shared_ptr<ImageTexture> texture;
    if (future.valid()) {
        texture = future.get();
    }

    if (usePlaceholder && !texture->isValid()) {
        future = ImageRegistry::get().getTexture(
            "tex-placeholder",
            placeholderPath,
            false,
            true,
            gammaCorrect,
            flipY,
            TextureType::diffuse,
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

std::string Material::resolveTexturePath(
    std::string_view textureName,
    bool compressed)
{
    if (textureName.empty()) return {};

    const auto& assets = Assets::get();

    std::pair<std::string, bool> texturePath{ "", false };

    if (!m_baseDir.empty()) {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        texturePath = selectTexturePath(
            util::joinPathExt(
                m_modelDir,
                m_baseDir,
                textureName,
                ""),
            compressed);
    }

    if (!texturePath.second) {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        texturePath = selectTexturePath(
            util::joinPathExt(
                m_modelDir,
                textureName,
                ""),
            compressed);
    }

    if (!texturePath.second && !m_baseDir.empty()) {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        texturePath = selectTexturePath(
            util::joinPathExt(
                m_baseDir,
                textureName,
                ""),
            compressed);
    }

    if (!texturePath.second && m_baseDir.empty()) {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        texturePath = selectTexturePath(
            textureName,
            compressed);
    }

    if (!texturePath.second) {
        KI_WARN_OUT(fmt::format(
            "TEX::MISSING: base_dir={}, name={}",
            m_baseDir,
            textureName));
    }
    else {
        KI_INFO_OUT(fmt::format(
            "TEX::FOUND: base_dir={}, name={}, path={}",
            m_baseDir,
            textureName,
            texturePath.first));
    }

    return texturePath.first;
}

// @param compressed use compressed if possible
void Material::addTexture(
    TextureType type,
    const std::string& path,
    bool compressed) noexcept
{
    if (path.empty()) {
        m_texturePaths.erase(type);
        KI_INFO_OUT(fmt::format("TEX::CLEAR: type={}, path={}", util::as_integer(type), path));
    }
    else {
        m_texturePaths[type] = { path, compressed };
    }
}

void Material::addinlineTexture(
    TextureType type,
    const std::shared_ptr<InlineTexture>& texture) noexcept
{
    m_inlineTextures.insert({ type, texture });
}

void Material::prepare()
{
    ASSERT_RT();

    if (m_prepared) return;
    m_prepared = true;

    for (auto& it : m_boundTextures) {
        auto& tex = it.second;
        tex.m_texture->prepare();
    }
}

const MaterialSSBO Material::toSSBO() const
{
    const auto& whitePx = ColorTexture::getWhiteRGBA().m_handle;
    const auto& blackPx = ColorTexture::getBlackRGBA().m_handle;

    uint8_t spritesY = spriteCount / spritesX;
    if (spriteCount % spritesX != 0) {
        spritesY++;
    }

    const glm::vec4 mrasFactor{
        m_occlusionFactor,
        m_metalnessFactor,
        m_roughnessFactor,
        0.f };

    return {
        kd,
        hasBoundTex(TextureType::emission) ? WHITE_RGBA : ke,

        hasBoundTex(TextureType::map_mras) ? mrasFactor : mras,

        getTexHandle(TextureType::diffuse, whitePx),
        getTexHandle(TextureType::emission, blackPx),

        getTexHandle(TextureType::map_normal, 0),

        getTexHandle(TextureType::map_opacity, whitePx),
        getTexHandle(TextureType::map_mras, whitePx),
        getTexHandle(TextureType::map_displacement, blackPx),

        getTexHandle(TextureType::map_dudv, 0),
        getTexHandle(TextureType::map_noise, 0),
        getTexHandle(TextureType::map_noise_2, 0),

        getTexHandle(TextureType::map_custom_1, 0),

        getFlags(),

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

unsigned int Material::getFlags() const
{
    unsigned int flags = 0;
    if (m_invertOcclusion) {
        flags |= MATERIAL_INVERT_OCCLUSION;
    }
    if (m_invertMetalness) {
        flags |= MATERIAL_INVERT_METALNESS;
    }
    if (m_invertRoughness) {
        flags |= MATERIAL_INVERT_ROUGHNESS;
    }
    return flags;
}
