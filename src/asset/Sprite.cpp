#include "Sprite.h"

#include <mutex>

#include <fmt/format.h>

#include "Shape.h"


namespace {
    int idBase = 0;

    std::mutex type_id_lock{};

    int nextID()
    {
        std::lock_guard<std::mutex> lock(type_id_lock);
        return ++idBase;
    }
}

Sprite::Sprite()
    : m_objectID(nextID())
{
}

Sprite::~Sprite()
{
    KI_INFO(fmt::format(
        "Sprite: delete - ID={}",
        m_objectID));
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
    const int objectID,
    std::vector<Sprite>& sprites)
{
    const auto& it = std::find_if(
        sprites.begin(),
        sprites.end(),
        [objectID](Sprite& m) { return m.m_objectID == objectID; });
    return it != sprites.end() ? &(*it) : nullptr;
}

const Sprite* Sprite::findID(
    const int objectID,
    const std::vector<Sprite>& sprites)
{
    const auto& it = std::find_if(
        sprites.begin(),
        sprites.end(),
        [objectID](const Sprite& m) { return m.m_objectID == objectID; });
    return it != sprites.end() ? &(*it) : nullptr;
}

void Sprite::prepare(const Assets& assets)
{
    if (m_prepared) return;
    m_prepared = true;
}
