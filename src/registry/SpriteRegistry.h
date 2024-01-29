#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <atomic>

#include "asset/Assets.h"
#include "asset/Sprite.h"
#include "asset/ShapeSSBO.h"

#include "kigl/GLBuffer.h"

struct UpdateContext;
class RenderContext;

class Sprite;
struct Shape;


class SpriteRegistry {
public:
    SpriteRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~SpriteRegistry();

    void registerSprite(Sprite& sprite);

    void prepare();

    void updateRT(const UpdateContext& ctx);

    void bind(
        const RenderContext& ctx);

private:
    void updateShapeBuffer();

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::atomic<bool> m_dirtyFlag;
    std::mutex m_lock{};

    std::vector<Sprite> m_sprites;
    std::map<ki::sprite_id, Sprite*> m_idToSprites;

    std::vector<ShapeSSBO> m_shapesSSBO;

    size_t m_shapeIndex{ 0 };

    size_t m_lastSpriteSize{ 0 };
    size_t m_lastShapeSize{ 0 };

    kigl::GLBuffer m_ssbo{ "shapes_ssbo" };
};
