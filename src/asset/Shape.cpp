#include "Shape.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <mutex>

#include "fmt/format.h"

#include "util/Util.h"

#include "asset/Shader.h"

#include "asset/ImageTexture.h"

#include "asset/TextureUBO.h"


namespace {
    TextureSpec textureSpec;
}

Shape::Shape()
{
}

Shape::~Shape()
{
    KI_INFO(fmt::format(
        "Shape: delete - index={}",
        m_registeredIndex));
}

void Shape::loadTextures(const Assets& assets)
{
    loadTexture(assets, SHAPE_DIFFUSE_IDX, map_kd, true, true);
    loadTexture(assets, SHAPE_EMISSION_IDX, map_ke, true, false);
    loadTexture(assets, SHAPE_SPECULAR_IDX, map_ks, true, false);
    loadTexture(assets, SHAPE_NORMAL_MAP_IDX, map_bump, false, false);
}

void Shape::loadTexture(
    const Assets& assets,
    int idx,
    const std::string& textureName,
    bool gammaCorrect,
    bool usePlaceholder)
{
    if (textureName.empty()) return;

    std::string texturePath = getTexturePath(assets, textureName);

    KI_INFO(fmt::format("Shape: texture={}", texturePath));

    std::string placeholderPath = assets.placeholderTexture;
    auto future = ImageTexture::getTexture(
        "shape-placeholder",
        assets.placeholderTextureAlways ? placeholderPath : texturePath,
        gammaCorrect,
        textureSpec);

    future.wait();

    ImageTexture* texture = { nullptr };
    if (future.valid()) {
        texture = future.get();
    }

    if (usePlaceholder && !texture->isValid()) {
        future = ImageTexture::getTexture("shape-placeholder", placeholderPath, gammaCorrect, textureSpec);
        future.wait();
        if (future.valid()) {
            texture = future.get();
        }
    }

    if (texture && texture->isValid()) {
        m_textures[idx].texture = texture;
    }
}

const std::string Shape::getTexturePath(
    const Assets& assets,
    const std::string& textureName)
{
    if (textureName.empty()) return {};

    std::string texturePath;
    {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        texturePath = util::joinPath(
            assets.spritesDir,
            textureName);
    }
    return texturePath;
}

bool Shape::hasTex(int index) const
{
    const auto& tex = m_textures[index];
    return tex.texture != nullptr;
}

void Shape::prepare(const Assets& assets)
{
    for (auto& tex : m_textures) {
        if (!tex.texture) continue;
        tex.texture->prepare(assets);
        tex.m_texIndex = tex.texture->m_texIndex;
        tex.m_handle = tex.texture->m_handle;
    }
}

const ShapeSSBO Shape::toSSBO() const
{
    for (auto& tex : m_textures) {
        if (!tex.texture) continue;
        ASSERT_TEX_INDEX(tex.m_texIndex);
    }

    return {
        m_textures[SHAPE_DIFFUSE_IDX].m_handle,
        m_textures[SHAPE_EMISSION_IDX].m_handle,
        //m_textures[SHAPE_SPECULAR_IDX].m_handle,
        m_textures[SHAPE_NORMAL_MAP_IDX].m_handle,
    };
}
