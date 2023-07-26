#include "Sprite.h"

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
    int idBase = 0;

    std::mutex type_id_lock{};

    TextureSpec textureSpec;

    int nextID()
    {
        std::lock_guard<std::mutex> lock(type_id_lock);
        return ++idBase;
    }
}

Sprite* Sprite::find(
    const std::string& name,
    std::vector<Sprite>& Sprites)
{
    const auto& it = std::find_if(
        Sprites.begin(),
        Sprites.end(),
        [&name](Sprite& m) { return m.m_name == name; });
    return it != Sprites.end() ? &(*it) : nullptr;
}

Sprite* Sprite::findID(
    const int objectID,
    std::vector<Sprite>& Sprites)
{
    const auto& it = std::find_if(
        Sprites.begin(),
        Sprites.end(),
        [objectID](Sprite& m) { return m.m_objectID == objectID; });
    return it != Sprites.end() ? &(*it) : nullptr;
}

const Sprite* Sprite::findID(
    const int objectID,
    const std::vector<Sprite>& Sprites)
{
    const auto& it = std::find_if(
        Sprites.begin(),
        Sprites.end(),
        [objectID](const Sprite& m) { return m.m_objectID == objectID; });
    return it != Sprites.end() ? &(*it) : nullptr;
}

Sprite::Sprite()
    : m_objectID(nextID())
{
}

Sprite::~Sprite()
{
    KI_INFO(fmt::format(
        "Sprite: delete - ID={}, name={}, index={}",
        m_objectID, m_name, m_registeredIndex));
}

void Sprite::loadTextures(const Assets& assets)
{
    if (m_loaded) return;
    m_loaded = true;

    loadTexture(assets, SPRITE_DIFFUSE_IDX, map_kd);
    loadTexture(assets, SPRITE_EMISSION_IDX, map_ke);
    loadTexture(assets, SPRITE_SPECULAR_IDX, map_ks);
    loadTexture(assets, SPRITE_NORMAL_MAP_IDX, map_bump);
}

void Sprite::loadTexture(
    const Assets& assets,
    int idx,
    const std::string& textureName)
{
    if (textureName.empty()) return;

    std::string texturePath = getTexturePath(assets, textureName);

    KI_INFO(fmt::format("Sprite: ID={} name={}, texture={}", m_objectID, m_name, texturePath));

    std::string placeholderPath = assets.placeholderTexture;
    auto future = ImageTexture::getTexture(
        assets.placeholderTextureAlways ? placeholderPath : texturePath,
        textureSpec);

    future.wait();

    ImageTexture* texture = { nullptr };
    if (future.valid()) {
        texture = future.get();
    }

    if (!texture->isValid()) {
        future = ImageTexture::getTexture(placeholderPath, textureSpec);
        future.wait();
        if (future.valid()) {
            texture = future.get();
        }
    }

    if (texture && texture->isValid()) {
        m_textures[idx].texture = texture;
    }
}

const std::string Sprite::getTexturePath(
    const Assets& assets,
    const std::string& textureName)
{
    if (textureName.empty()) return {};

    std::string texturePath;
    {
        std::filesystem::path fp;
        fp /= assets.spritesDir;
        fp /= m_path;
        fp /= textureName;
        texturePath = fp.string();
    }
    return texturePath;
}

bool Sprite::hasTex(int index) const
{
    const auto& tex = m_textures[index];
    return tex.texture != nullptr;
}

void Sprite::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    for (auto& tex : m_textures) {
        if (!tex.texture) continue;
        tex.texture->prepare(assets);
        tex.m_texIndex = tex.texture->m_texIndex;
    }
}

const SpriteSSBO Sprite::toSSBO() const
{
    for (auto& tex : m_textures) {
        if (!tex.texture) continue;
        ASSERT_TEX_INDEX(tex.m_texIndex);
    }

    return {
        m_textures[SPRITE_DIFFUSE_IDX].m_texIndex,
        m_textures[SPRITE_EMISSION_IDX].m_texIndex,
        m_textures[SPRITE_SPECULAR_IDX].m_texIndex,
        m_textures[SPRITE_NORMAL_MAP_IDX].m_texIndex,
    };
}
