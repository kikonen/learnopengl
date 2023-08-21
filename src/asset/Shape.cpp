#include "Shape.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <mutex>

#include "fmt/format.h"

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
    loadTexture(assets, SHAPE_DIFFUSE_IDX, map_kd);
    loadTexture(assets, SHAPE_EMISSION_IDX, map_ke);
    loadTexture(assets, SHAPE_SPECULAR_IDX, map_ks);
    loadTexture(assets, SHAPE_NORMAL_MAP_IDX, map_bump);
}

void Shape::loadTexture(
    const Assets& assets,
    int idx,
    const std::string& textureName)
{
    if (textureName.empty()) return;

    std::string texturePath = getTexturePath(assets, textureName);

    KI_INFO(fmt::format("Shape: texture={}", texturePath));

    std::string placeholderPath = assets.placeholderTexture;
    auto future = ImageTexture::getTexture(
        "shape-placeholder",
        assets.placeholderTextureAlways ? placeholderPath : texturePath,
        textureSpec);

    future.wait();

    ImageTexture* texture = { nullptr };
    if (future.valid()) {
        texture = future.get();
    }

    if (!texture->isValid()) {
        future = ImageTexture::getTexture("shape-placeholder", placeholderPath, textureSpec);
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
        std::filesystem::path fp;
        fp /= assets.spritesDir;
        fp /= textureName;
        texturePath = fp.string();
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
    }
}

const ShapeSSBO Shape::toSSBO() const
{
    for (auto& tex : m_textures) {
        if (!tex.texture) continue;
        ASSERT_TEX_INDEX(tex.m_texIndex);
    }

    return {
        m_textures[SHAPE_DIFFUSE_IDX].m_texIndex,
        m_textures[SHAPE_EMISSION_IDX].m_texIndex,
        m_textures[SHAPE_SPECULAR_IDX].m_texIndex,
        m_textures[SHAPE_NORMAL_MAP_IDX].m_texIndex,
    };
}
