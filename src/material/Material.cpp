#include "Material.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "fmt/format.h"

#include "pool/IdGenerator.h"

#include "util/util.h"
#include "util/file.h"
#include "util/thread.h"

#include "asset/Assets.h"

#include "shader/Shader.h"

#include "ImageTexture.h"
#include "ChannelTexture.h"
#include "ColorTexture.h"
#include "MaterialSSBO.h"
#include "MaterialRegistry.h"
#include "MaterialUpdater.h"

namespace {
    IdGenerator<ki::material_id> ID_GENERATOR;

    const glm::vec4 WHITE_RGBA{ 1.f };
    const glm::vec4 BLACK_RGBA{ 0.f };

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

    std::string selectTexturePath(
        std::string_view path,
        bool useCompressed)
    {
        const auto& assets = Assets::get();

        std::filesystem::path filePath;

        bool found = false;

        if (useCompressed && assets.compressedTexturesEnabled) {
            std::filesystem::path ktxPath{ path };
            ktxPath.replace_extension(".ktx");

            const auto fullPath = util::joinPath(
                assets.assetsBuildDir,
                ktxPath.string());

            if (util::fileExists(fullPath)) {
                KI_INFO_OUT(fmt::format("FOUND: ktx_path={}", fullPath));
                filePath = fullPath;
                found = true;
            }
        }

        if (!found) {
            const auto fullPath = util::joinPath(
                assets.assetsBuildDir,
                path);

            if (util::fileExists(fullPath)) {
                filePath = fullPath;
                found = true;
            }
        }

        if (!found) {
            filePath = util::joinPath(assets.assetsDir, path);
        }

        return filePath.string();
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
Material::Material(Material&& o) = default;

Material::~Material() = default;
//{
//    //KI_INFO(fmt::format(
//    //    "MATERIAL: delete - ID={}, name={}, index={}",
//    //    m_id, m_name, m_registeredIndex));
//}

Material& Material::operator=(const Material& o) = default;
Material& Material::operator=(Material&& o) = default;

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
        bool gammaCorrect = false;
        bool flipY = true;
        bool usePlaceholder = false;

        if (type == TextureType::diffuse) {
            gammaCorrect = true;
            usePlaceholder = true;
        }
        else if (type == TextureType::emission) {
            gammaCorrect = true;
        }

        loadTexture(type, gammaCorrect, flipY, usePlaceholder);
    }

    if (!hasRegisteredTex(TextureType::map_metal))
    {
        loadChannelTexture(
            TextureType::map_metal,
            fmt::format("material_{}_metal", m_registeredIndex),
            map_channelParts,
            metal);
    }
}

void Material::loadTexture(
    TextureType type,
    bool gammaCorrect,
    bool flipY,
    bool usePlaceholder)
{
    const auto& it = m_texturePaths.find(type);
    if (it == m_texturePaths.end()) return;

    const auto& info = it->second;

    const auto& assets = Assets::get();

    std::string texturePath = resolveTexturePath(info.path, info.compressed);

    KI_INFO(fmt::format("MATERIAL: ID={}, name={}, texture={}", m_id, m_name, texturePath));

    const std::string& placeholderPath = util::joinPath(assets.assetsDir, assets.placeholderTexture);

    auto future = ImageTexture::getTexture(
        info.path,
        usePlaceholder && assets.placeholderTextureAlways ? placeholderPath : texturePath,
        gammaCorrect,
        flipY,
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
            flipY,
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
    const std::vector<ChannelPart>& parts,
    const glm::vec4& defaults)
{
    const auto& assets = Assets::get();

    std::vector<ImageTexture*> sourceTextures;

    int validCount = 0;
    for (const auto& part : parts) {
        const auto& it = m_boundTextures.find(part.m_type);

        auto* bound = it != m_boundTextures.end() ? &it->second : nullptr;
        if (bound) {
            sourceTextures.push_back((ImageTexture*)bound->m_texture);
            bound->m_channelPart = true;
            validCount++;
        }
        else {
            sourceTextures.push_back(nullptr);
        }
    }

    KI_INFO(fmt::format("MATERIAL: ID={}, name={}, texture={}, validCount={}", m_id, m_name, name, validCount));

    if (validCount == 0) return;

    auto future = ChannelTexture::getTexture(
        name,
        parts,
        sourceTextures,
        defaults,
        4,
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

std::string Material::resolveTexturePath(
    std::string_view textureName,
    bool compressed)
{
    if (textureName.empty()) return {};

    const auto& assets = Assets::get();

    std::string texturePath;

    if (!m_baseDir.empty())
    {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        texturePath = selectTexturePath(
            util::joinPathExt(
                m_modelDir,
                m_baseDir,
                textureName,
                ""),
            compressed);
    }

    if (!util::fileExists(texturePath))
    {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        texturePath = selectTexturePath(
            util::joinPathExt(
                m_modelDir,
                textureName,
                ""),
            compressed);
    }

    if (!util::fileExists(texturePath) && !m_baseDir.empty())
    {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        texturePath = selectTexturePath(
            util::joinPathExt(
                m_baseDir,
                textureName,
                ""),
            compressed);
    }

    if (!util::fileExists(texturePath) && m_baseDir.empty())
    {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        texturePath = selectTexturePath(
            textureName,
            compressed);
    }

    return texturePath;
}

void Material::prepare()
{
    ASSERT_RT();

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
    const auto& blackPx = ColorTexture::getBlackRGBA().m_handle;

    uint8_t spritesY = spriteCount / spritesX;
    if (spriteCount % spritesX != 0) {
        spritesY++;
    }

    return {
        kd,
        hasBoundTex(TextureType::emission) ? WHITE_RGBA : ke,

        hasBoundTex(TextureType::map_metal) ? WHITE_RGBA : metal,

        getTexHandle(TextureType::diffuse, whitePx),
        getTexHandle(TextureType::emission, blackPx),

        getTexHandle(TextureType::map_normal, 0),
        getTexHandle(TextureType::map_dudv, 0),
        getTexHandle(TextureType::map_noise, 0),
        getTexHandle(TextureType::map_noise_2, 0),

        getTexHandle(TextureType::map_opacity, whitePx),
        getTexHandle(TextureType::map_custom_1, 0),
        getTexHandle(TextureType::map_metal, whitePx),

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
