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
#include "shader/ProgramRegistry.h"

#include "material/ImageRegistry.h"

#include "ImageTexture.h"
#include "InlineTexture.h"
#include "ColorTexture.h"

#include "MaterialSSBO.h"
#include "MaterialRegistry.h"
#include "MaterialUpdater.h"

namespace {
    IdGenerator<ki::material_id> ID_GENERATOR;

    const util::Ref<Material> NULL_MATERIAL{};

    constexpr unsigned int MATERIAL_INVERT_OCCLUSION = 1;
    constexpr unsigned int MATERIAL_INVERT_METALNESS = 2;
    constexpr unsigned int MATERIAL_INVERT_ROUGHNESS = 4;
    constexpr unsigned int MATERIAL_SCALE_TILING = 8;

    const glm::vec4 WHITE_RGBA{ 1.f };
    const glm::vec4 BLACK_RGBA{ 0.f };

    const std::regex CONTAINS_BUILD = std::regex(".*_build.*");

    //float calculateAmbient(glm::vec3 ambient) {
    //    return (ambient.x + ambient.y + ambient.z) / 3.f;
    //}

    util::Ref<Material> createDefaultMaterial() {
        auto mat = util::Ref<Material>::create();
        mat->m_name = "<default>";
        mat->kd = glm::vec4(0.8f, 0.8f, 0.0f, 1.f);
        return mat;
    }

    util::Ref<Material> createBasicMaterial() {
        auto mat = util::Ref<Material>::create();
        mat->m_name = "<basic>";
        mat->kd = glm::vec4(0.8f, 0.8f, 0.0f, 1.f);
        return mat;
    }

    util::Ref<Material> createRGBMaterial(std::string_view name, const glm::vec4& color) {
        auto mat = util::Ref<Material>::create();
        mat->m_name = name;
        mat->kd = color;
        return mat;
    }

    util::Ref<Material> createGoldMaterial() {
        auto mat = util::Ref<Material>::create();
        mat->m_name = "<gold>";
        mat->kd = glm::vec4(0.7516f, 0.6065f, 0.2265f, 1.f);
        return mat;
    }

    util::Ref<Material> createSilverMaterial() {
        auto mat = util::Ref<Material>::create();
        mat->m_name = "<silver>";
        mat->kd = glm::vec4(0.5075f, 0.5075f, 0.5075f, 1.f);
        return mat;
    }

    util::Ref<Material> createBronzeMaterial() {
        auto mat = util::Ref<Material>::create();
        mat->m_name = "<bronze>";
        mat->kd = glm::vec4(0.7140f, 0.4284f, 0.1814f, 1.f);
        return mat;
    }

    util::Ref<Material> createHighlightMaterial() {
        auto mat = util::Ref<Material>::create();
        mat->m_name = "<highlight>";
        mat->kd = glm::vec4(0.0f, 0.0f, 0.8f, 1.f);
        return mat;
    }

    util::Ref<Material> createSelectionMaterial() {
        auto mat = util::Ref<Material>::create();
        mat->m_name = "<selection>";
        mat->kd = glm::vec4(0.8f, 0.0f, 0.0f, 1.f);
        return mat;
    }

    util::Ref<Material> createWireframeMaterial() {
        auto mat = util::Ref<Material>::create();
        mat->m_name = "<wireframe>";
        mat->kd = glm::vec4(0.0f, 0.8f, 0.0f, 1.f);
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

    inline uint32_t packSprites(uint32_t count, uint32_t spritesX, uint32_t spritesY) {
        // Validate at pack time — silent truncation here is a corruption bug that
        // only shows as wrong sprite UVs much later.
        assert(count <= 0xFFFFu && "spriteCount exceeds 16-bit packing budget");
        assert(spritesX <= 0xFFu && "spritesX exceeds 8-bit packing budget");
        assert(spritesY <= 0xFFu && "spritesY exceeds 8-bit packing budget");
        return (count << 16) | ((spritesX & 0xFFu) << 8) | (spritesY & 0xFFu);
    }

    inline uint32_t unpackSpriteCount(uint32_t packed) { return  packed >> 16; }
    inline uint32_t unpackSpritesX(uint32_t packed) { return (packed >> 8) & 0xFFu; }
    inline uint32_t unpackSpritesY(uint32_t packed) { return  packed & 0xFFu; }

    inline uint32_t packSprites(const Material& material) {
        uint8_t spritesY = material.spriteCount / material.spritesX;
        if (material.spriteCount % material.spritesX != 0) {
            spritesY++;
        }

        return packSprites(material.spriteCount, material.spritesX, spritesY);
    }
}

util::Ref<Material> Material::createMaterial(BasicMaterial type)
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

const util::Ref<Material>& Material::find(
    std::string_view name,
    std::vector<util::Ref<Material>>& materials)
{
    const auto& it = std::find_if(
        materials.begin(),
        materials.end(),
        [&name](const util::Ref<Material>& m) { return m->m_name == name; });
    return it != materials.end() ? *it : NULL_MATERIAL;
}

//util::Ref<Material> Material::findID(
//    const ki::material_id id,
//    std::vector<util::Ref<Material>>& materials)
//{
//    const auto& it = std::find_if(
//        materials.begin(),
//        materials.end(),
//        [id](Material& m) { return m.m_id == id; });
//    return it != materials.end() ? &(*it) : nullptr;
//}
//
//const util::Ref<Material> Material::findID(
//    const ki::material_id id,
//    const std::vector<util::Ref<Material>>& materials)
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

//Material::Material(Material& o) = default;
//Material::Material(const Material& o) = default;

//Material::Material(Material&& o) noexcept = default;
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

Material& Material::operator=(const Material& o)
{
    if (&o == this) return *this;

    // NOTE KI Keep identity
    //m_id = o.m_id
    //m_refCount = o.m_refCount;

    m_registeredIndex = o.m_registeredIndex;

    textureSpec = o.textureSpec;

    pattern = o.pattern;
    reflection = o.reflection;
    refraction = o.refraction;
    refractionRatio = refractionRatio;

    tilingX = o.tilingX;
    tilingY = o.tilingY;

    map_bump_strength = o.map_bump_strength;

    kd = o.kd;
    ke = o.ke;

    mras = o.mras;

    m_occlusionFactor = o.m_occlusionFactor;
    m_metalnessFactor = o.m_metalnessFactor;
    m_roughnessFactor = o.m_roughnessFactor;

    m_invertOcclusion = o.m_invertOcclusion;
    m_invertMetalness = o.m_invertMetalness;
    m_invertRoughness = o.m_invertRoughness;

    m_scaleTiling = o.m_scaleTiling;

    pointSize = o.pointSize;

    layers = o.layers;
    layersDepth = o.layersDepth;
    parallaxDepth = o.parallaxDepth;

    m_name = o.m_name;

    spriteCount = o.spriteCount;
    spritesX = o.spritesX;

    alpha = o.alpha;
    blend = o.blend;

    renderBack = o.renderBack;
    lineMode = o.lineMode;
    reverseFrontFace = o.reverseFrontFace;
    noDepth = o.noDepth;

    useDeferred = o.useDeferred;
    inmutable = o.inmutable;

    usePreDepth = o.usePreDepth;
    useJoints = o.useJoints;

    m_geometryType = o.m_geometryType;
    m_baseDir = o.m_baseDir;
    m_modelDir = o.m_modelDir;

    m_defaultPrograms = o.m_defaultPrograms;
    m_programNames = o.m_programNames;

    m_sharedDefinitions = o.m_sharedDefinitions;
    m_programDefinitions = o.m_programDefinitions;
    m_oitDefinitions = o.m_oitDefinitions;
    m_shadowDefinitions = o.m_shadowDefinitions;
    m_selectionDefinitions = o.m_selectionDefinitions;
    m_objectIdDefinitions = o.m_objectIdDefinitions;
    m_normalDefinitions = o.m_normalDefinitions;

    m_programs = o.m_programs;

    m_updaterId = o.m_updaterId;

    m_updater = o.m_updater;

    m_fontAtlasTex = o.m_fontAtlasTex;

    m_boundTextures = o.m_boundTextures;
    m_texturePaths = o.m_texturePaths;
    m_inlineTextures = o.m_inlineTextures;

    m_textureTransforms = o.m_textureTransforms;

    m_prepared = o.m_prepared;
    m_loaded = o.m_loaded;

    return *this;
}

//Material& Material::operator=(Material&& o) noexcept = default;

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
    return MaterialRegistry::get().registerMaterial(this);
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

    util::Ref<ImageTexture> texture;
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
    const util::Ref<InlineTexture>& texture) noexcept
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

//const MaterialSSBO Material::toSSBO() const
//{
//    const auto& whitePx = ColorTexture::getWhiteRGBA().m_handle;
//    const auto& blackPx = ColorTexture::getBlackRGBA().m_handle;
//    const auto& flatNormalPx = ColorTexture::getFlatNormalRGBA().m_handle;
//
//    // RGB8 = (128, 128, 255) = flat normal
//    uint8_t flatNormal[] = { 128, 128, 255 };
//
//    const glm::vec4 mrasFactor{
//        m_metalnessFactor,
//        m_occlusionFactor,
//        m_roughnessFactor,
//        1.f };
//
//    return {
//        kd,
//        hasBoundTex(TextureType::emission) ? WHITE_RGBA : ke,
//
//        hasBoundTex(TextureType::map_mras) ? mrasFactor : mras,
//
//        getTexHandle(TextureType::diffuse, whitePx),
//        getTexHandle(TextureType::emission, blackPx),
//
//        getTexHandle(TextureType::map_normal, flatNormalPx),
//
//        getTexHandle(TextureType::map_opacity, whitePx),
//        // NOTE KI whitePx fails due to "inverse" flags
//        getTexHandle(TextureType::map_mras, 0),
//        getTexHandle(TextureType::map_displacement, blackPx),
//
//        getTexHandle(TextureType::map_dudv, 0),
//        getTexHandle(TextureType::map_noise, 0),
//        getTexHandle(TextureType::map_noise_2, 0),
//
//        getTexHandle(TextureType::map_custom_1, 0),
//
//        getFlags(),
//
//        reflection,
//        refraction,
//        getRefractionRatio(),
//
//        tilingX,
//        tilingY,
//
//        packSprites(*this),
//
//        layers,
//        layersDepth,
//        parallaxDepth,
//        pointSize,
//    };
//}

void Material::fillSSBO(
    MaterialMainSSBO& main,
    MaterialCustomSSBO& custom,
    MaterialColdSSBO& cold) const
{
    const auto& whitePx = ColorTexture::getWhiteRGBA().m_handle;
    const auto& blackPx = ColorTexture::getBlackRGBA().m_handle;
    const auto& flatNormalPx = ColorTexture::getFlatNormalRGBA().m_handle;

    // RGB8 = (128, 128, 255) = flat normal
    uint8_t flatNormal[] = { 128, 128, 255 };

    const glm::vec4 mrasFactor{
        m_metalnessFactor,
        m_occlusionFactor,
        m_roughnessFactor,
        1.f };

    main = {
        .u_diffuse = kd,
        .u_emission = hasBoundTex(TextureType::emission) ? WHITE_RGBA : ke,
        .u_mras = hasBoundTex(TextureType::map_mras) ? mrasFactor : mras,

        .u_diffuseTex = getTexHandle(TextureType::diffuse, whitePx),
        .u_emissionTex = getTexHandle(TextureType::emission, blackPx),
        .u_normalMap = getTexHandle(TextureType::map_normal, flatNormalPx),
        .u_opacityMap = getTexHandle(TextureType::map_opacity, whitePx),
        // NOTE KI whitePx fails due to "inverse" flags
        .u_mrasMap = getTexHandle(TextureType::map_mras, 0),

        .u_flags = getFlags(),

        .u_tilingX = tilingX,
        .u_tilingY = tilingY,

        .u_parallaxDepth = parallaxDepth,
    };

    custom = {
        .u_displacementMap = getTexHandle(TextureType::map_displacement, blackPx),

        .u_dudvMap = getTexHandle(TextureType::map_dudv, 0),
        .u_noiseMap = getTexHandle(TextureType::map_noise, 0),
        .u_noise2Map = getTexHandle(TextureType::map_noise_2, 0),

        .u_custom1Map = getTexHandle(TextureType::map_custom_1, 0),

        .u_fontHAtlas = m_fontAtlasTex,
    };
    cold = {
        .u_reflection = reflection,
        .u_refraction = refraction,
        .u_refractionRatio = getRefractionRatio(),

        .u_packedSprites = packSprites(*this),

        .u_layers = layers,
        .u_layersDepth = layersDepth,
        .u_pointSize = pointSize,
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
    if (m_scaleTiling) {
        flags |= MATERIAL_SCALE_TILING;
    }
    return flags;
}

void Material::resolveMaterial()
{
    Material& material = *this;

    {
        const auto& shaderName = selectProgram(
            MaterialProgramType::shader,
            material.m_programNames,
            material.m_defaultPrograms ? SHADER_G_TEX : "");

        if (shaderName.starts_with("g_")) {
            material.useDeferred = true;
        }

        if (material.blend) {
            // NOTE KI alpha MUST BE true if blend
            material.alpha = true;
        }
    }

    material.loadTextures();
    resolveProgram();

    {
        bool useParallax = material.hasBoundTex(TextureType::map_displacement) && material.parallaxDepth > 0;
        if (!useParallax) {
            material.parallaxDepth = 0.f;
        }
    }
}

void Material::resolveProgram()
{
    Material& material = *this;

    const auto& assets = Assets::get();

    const bool useDudvTex = material.hasBoundTex(TextureType::map_dudv);
    const bool useDisplacementTex = material.hasBoundTex(TextureType::map_displacement);
    const bool useNormalTex = material.hasBoundTex(TextureType::map_normal);
    const bool useCubeMap = 1.0 - material.reflection - material.refraction < 1.0;
    const bool useNormalPattern = material.pattern > 0;
    const bool useParallax = material.hasBoundTex(TextureType::map_displacement) && material.parallaxDepth > 0;

    const bool useTBN = useNormalTex || useDudvTex || useDisplacementTex;

    const auto& shaderName = selectProgram(
        MaterialProgramType::shader,
        material.m_programNames,
        material.m_defaultPrograms ? SHADER_G_TEX : "");

    auto preDepthName = selectProgram(
        MaterialProgramType::pre_depth,
        material.m_programNames,
        SHADER_PRE_DEPTH_PASS);

    const auto& oitName = selectProgram(
        MaterialProgramType::oit,
        material.m_programNames,
        material.m_defaultPrograms ? SHADER_OIT_PASS : "");

    const auto& shadowName = selectProgram(
        MaterialProgramType::shadow,
        material.m_programNames,
        material.m_defaultPrograms ? SHADER_SHADOW : "");

    const auto& selectionName = selectProgram(
        MaterialProgramType::selection,
        material.m_programNames,
        SHADER_SELECTION);

    const auto& objectIdName = selectProgram(
        MaterialProgramType::object_id,
        material.m_programNames,
        SHADER_OBJECT_ID);

    const auto& normalName = selectProgram(
        MaterialProgramType::normal,
        material.m_programNames,
        SHADER_NORMAL);

    if (!shaderName.empty()) {
        std::map<std::string, std::string, std::less<>> definitions;
        std::map<std::string, std::string, std::less<>> oitDefinitions;
        std::map<std::string, std::string, std::less<>> shadowDefinitions;
        std::map<std::string, std::string, std::less<>> selectionDefinitions;
        std::map<std::string, std::string, std::less<>> objectIdDefinitions;
        std::map<std::string, std::string, std::less<>> normalDefinitions;

        for (const auto& [k, v] : material.m_sharedDefinitions) {
            definitions[k] = v;
            oitDefinitions[k] = v;
            shadowDefinitions[k] = v;
            selectionDefinitions[k] = v;
            objectIdDefinitions[k] = v;
            normalDefinitions[k] = v;
        }

        for (const auto& [k, v] : material.m_programDefinitions) {
            definitions[k] = v;
        }

        for (const auto& [k, v] : material.m_oitDefinitions) {
            oitDefinitions[k] = v;
        }

        // NOTE KI *NOT* same as program, to allow maximal reuse of shadow program
        // i.e. most defs don't affect shadow
        for (const auto& [k, v] : material.m_shadowDefinitions) {
            shadowDefinitions[k] = v;
        }

        for (const auto& [k, v] : material.m_selectionDefinitions) {
            selectionDefinitions[k] = v;
        }

        for (const auto& [k, v] : material.m_objectIdDefinitions) {
            objectIdDefinitions[k] = v;
        }

        for (const auto& [k, v] : material.m_normalDefinitions) {
            normalDefinitions[k] = v;
        }

        std::map<std::string, std::string, std::less<>> preDepthDefinitions;

        const bool useDebug = assets.glslUseDebug;
        bool usePreDepth = material.usePreDepth;
        bool useJoints = material.useJoints;

        if (material.alpha) {
            definitions[DEF_USE_ALPHA] = "1";
            //oitDefinitions[DEF_USE_ALPHA] = "1";
            shadowDefinitions[DEF_USE_ALPHA] = "1";
            selectionDefinitions[DEF_USE_ALPHA] = "1";
            objectIdDefinitions[DEF_USE_ALPHA] = "1";
            usePreDepth = false;
        }
        if (material.blend) {
            definitions[DEF_USE_BLEND] = "1";
            usePreDepth = false;
        }

        if (useTBN) {
            definitions[DEF_USE_TBN] = "1";
        }
        //if (useDudvTex) {
        //    definitions[DEF_USE_DUDV_TEX] = "1";
        //}
        //if (useDisplacementTex) {
        //    definitions[DEF_USE_DISPLACEMENT_TEX] = "1";
        //}
        if (useNormalTex) {
            definitions[DEF_USE_NORMAL_TEX] = "1";
        }
        if (useParallax) {
            definitions[DEF_USE_PARALLAX] = "1";
        }
        if (useCubeMap) {
            definitions[DEF_USE_CUBE_MAP] = "1";
        }
        if (useNormalPattern) {
            definitions[DEF_USE_NORMAL_PATTERN] = "1";
        }
        if (useJoints) {
            definitions[DEF_USE_JOINTS] = "1";
            oitDefinitions[DEF_USE_JOINTS] = "1";
            shadowDefinitions[DEF_USE_JOINTS] = "1";
            selectionDefinitions[DEF_USE_JOINTS] = "1";
            objectIdDefinitions[DEF_USE_JOINTS] = "1";
            normalDefinitions[DEF_USE_JOINTS] = "1";
        }
        //if (useSockets) {
        //    definitions[DEF_USE_SOCKETS] = "1";
        //    oitDefinitions[DEF_USE_SOCKETS] = "1";
        //    shadowDefinitions[DEF_USE_SOCKETS] = "1";
        //    selectionDefinitions[DEF_USE_SOCKETS] = "1";
        //    idDefinitions[DEF_USE_SOCKETS] = "1";
        //    normalDefinitions[DEF_USE_SOCKETS] = "1";
        //}
        if (useDebug) {
            definitions[DEF_USE_DEBUG] = "1";
        }

        material.m_programs[MaterialProgramType::shader] = ProgramRegistry::get().getProgramId(
            shaderName,
            false,
            material.m_geometryType,
            definitions);

        if (!oitName.empty()) {
            material.m_programs[MaterialProgramType::oit] = ProgramRegistry::get().getProgramId(
                oitName,
                false,
                "",
                oitDefinitions);
        }

        if (!shadowName.empty()) {
            //{
            //    size_t shadowCount = std::min(
            //        std::max(Assets::get().shadowPlanes.size() - 1, static_cast<size_t>(1)),
            //        static_cast<size_t>(MAX_SHADOW_MAP_COUNT_ABS));

            //    shadowDefinitions[DEF_MAX_SHADOW_MAP_COUNT] = std::to_string(shadowCount);
            //}

            material.m_programs[MaterialProgramType::shadow] = ProgramRegistry::get().getProgramId(
                shadowName,
                false,
                "",
                shadowDefinitions);
        }

        if (usePreDepth) {
            material.m_programs[MaterialProgramType::pre_depth] = ProgramRegistry::get().getProgramId(
                preDepthName,
                false,
                "",
                preDepthDefinitions);
        }

        if (!selectionName.empty()) {
            material.m_programs[MaterialProgramType::selection] = ProgramRegistry::get().getProgramId(
                selectionName,
                false,
                "",
                selectionDefinitions);
        }

        if (!objectIdName.empty()) {
            material.m_programs[MaterialProgramType::object_id] = ProgramRegistry::get().getProgramId(
                objectIdName,
                false,
                "",
                objectIdDefinitions);
        }

        if (!normalName.empty()) {
            material.m_programs[MaterialProgramType::normal] = ProgramRegistry::get().getProgramId(
                normalName,
                false,
                "",
                normalDefinitions);
        }
    }
}

std::string Material::selectProgram(
    MaterialProgramType type,
    const std::map<MaterialProgramType, std::string> programs,
    const std::string& defaultValue)
{
    std::string program;
    bool found = false;
    const auto& it = programs.find(type);
    if (it != programs.end()) {
        program = it->second;
        found = true;
    }
    return found ? program : defaultValue;
}
