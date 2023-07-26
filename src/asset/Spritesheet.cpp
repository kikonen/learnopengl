#include "SpriteSheet.h"

#include <fmt/format.h>

#include "Sprite.h"


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

SpriteSheet::SpriteSheet()
    : m_objectID(nextID())
{
}

SpriteSheet::~SpriteSheet()
{
    KI_INFO(fmt::format(
        "Spritesheet: delete - ID={}",
        m_objectID));
}

void SpriteSheet::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;

    for (auto& sprite : m_sprites) {
        sprite.prepare(assets);
        sprite.loadTextures(assets);
    }
}
