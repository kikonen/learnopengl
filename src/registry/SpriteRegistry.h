#pragma once

#include <vector>

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

    // Updates m_registeredIndex of Sprite
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

    std::vector<ShapeSSBO> m_shapesSSBO;

    size_t m_shapeIndex = 0;

    size_t m_lastSpriteSize = 0;

    GLBuffer m_ssbo{ "shapesSSBO" };
};
