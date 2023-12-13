#pragma once

#include <vector>
#include <map>

#include "asset/Assets.h"
#include "asset/Sprite.h"
#include "asset/ShapeSSBO.h"

#include "kigl/GLBuffer.h"

class UpdateContext;
class RenderContext;

class Sprite;
struct Shape;


class SpriteRegistry {
public:
    SpriteRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~SpriteRegistry() = default;

    void add(Sprite& sprite);

    void prepare();

    void update(const UpdateContext& ctx);

    void bind(
        const RenderContext& ctx);

private:
    void updateShapeBuffer();

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::vector<Sprite> m_sprites;
    std::map<ki::sprite_id, Sprite*> m_idToSprites;

    std::vector<ShapeSSBO> m_shapesSSBO;

    size_t m_shapeIndex{ 0 };

    size_t m_lastSpriteSize{ 0 };
    size_t m_lastShapeSize{ 0 };

    GLBuffer m_ssbo{ "shapesSSBO" };
};
