#include "Sprite.h"

#include <mutex>

#include <fmt/format.h>

#include "Shape.h"


namespace {
    ki::sprite_id idBase = 0;

    std::mutex type_id_lock{};

    ki::sprite_id nextID()
    {
        std::lock_guard<std::mutex> lock(type_id_lock);
        return ++idBase;
    }
}

Sprite::Sprite()
    : m_id(nextID())
{
}

Sprite::~Sprite()
{
    KI_INFO(fmt::format(
        "Sprite: delete - ID={}",
        m_id));
}

Sprite* Sprite::find(
    std::string_view name,
    std::vector<Sprite>& sprites)
{
    const auto& it = std::find_if(
        sprites.begin(),
        sprites.end(),
        [&name](Sprite& m) { return m.m_name == name; });
    return it != sprites.end() ? &(*it) : nullptr;
}

Sprite* Sprite::findID(
    const ki::sprite_id id,
    std::vector<Sprite>& sprites)
{
    const auto& it = std::find_if(
        sprites.begin(),
        sprites.end(),
        [id](Sprite& m) { return m.m_id == id; });
    return it != sprites.end() ? &(*it) : nullptr;
}

const Sprite* Sprite::findID(
    const ki::sprite_id id,
    const std::vector<Sprite>& sprites)
{
    const auto& it = std::find_if(
        sprites.begin(),
        sprites.end(),
        [id](const Sprite& m) { return m.m_id == id; });
    return it != sprites.end() ? &(*it) : nullptr;
}

void Sprite::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;
}
