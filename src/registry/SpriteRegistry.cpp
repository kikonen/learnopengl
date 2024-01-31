#include "SpriteRegistry.h"

#include "fmt/format.h"

#include "asset/Sprite.h"
#include "asset/Shape.h"
#include "asset/SSBO.h"
#include "asset/ShapeSSBO.h"


namespace {
    constexpr size_t SHAPE_BLOCK_SIZE = 10;
    constexpr size_t SHAPE_BLOCK_COUNT = 1000;

    constexpr size_t MAX_SHAPE_COUNT = SHAPE_BLOCK_SIZE * SHAPE_BLOCK_COUNT;
}

SpriteRegistry::SpriteRegistry(
    const Assets& assets)
    : m_assets(assets)
{
    // HACK KI reserve nax to avoid memory alloc issue main vs. worker
    m_shapesSSBO.reserve(MAX_SHAPE_COUNT);
    Shape shape;
    m_shapesSSBO.emplace_back(shape.toSSBO());
    m_shapeIndex++;
}

SpriteRegistry::~SpriteRegistry() = default;

void SpriteRegistry::registerSprite(Sprite& sprite)
{
    std::lock_guard lock(m_lock);

    if (const auto& it = m_idToSprites.find(sprite.m_id);
        it != m_idToSprites.end())
    {
        auto& ref = it->second;
        for (int i = 0; i < ref->m_shapes.size(); i++) {
            sprite.m_shapes[i].m_registeredIndex = ref->m_shapes[i].m_registeredIndex;
        }
        return;
    }

    const size_t count = sprite.m_shapes.size();


    if (m_shapeIndex + count > MAX_SHAPE_COUNT)
        throw std::runtime_error{ fmt::format("MAX_SHAPE_COUNT: {}", MAX_SHAPE_COUNT) };

    {
        size_t size = m_shapeIndex + std::max(SHAPE_BLOCK_SIZE, count) + SHAPE_BLOCK_SIZE;
        size += SHAPE_BLOCK_SIZE - size % SHAPE_BLOCK_SIZE;
        size = std::min(size, MAX_SHAPE_COUNT);
        m_shapesSSBO.reserve(size);
    }

    for (auto& shape : sprite.m_shapes) {
        shape.m_registeredIndex = (int)m_shapeIndex++;
    }

    auto& ref = m_sprites.emplace_back(sprite);
    m_idToSprites[sprite.m_id] = &ref;
}

void SpriteRegistry::updateRT(const UpdateContext& ctx)
{
    //if (!m_dirty) return;
    std::lock_guard lock(m_lock);

    updateShapeBuffer();
}

void SpriteRegistry::prepare()
{
    m_ssbo.createEmpty(SHAPE_BLOCK_SIZE * sizeof(ShapeSSBO), GL_DYNAMIC_STORAGE_BIT);
}

void SpriteRegistry::bind(
    const RenderContext& ctx)
{
    m_ssbo.bindSSBO(SSBO_SHAPES);
}

void SpriteRegistry::updateShapeBuffer()
{
    const size_t spriteIndex = m_lastSpriteSize;
    const size_t spriteCount = m_sprites.size();
    const size_t shapeIndex = m_lastShapeSize;

    if (spriteIndex == spriteCount) return;
    if (spriteCount == 0) return;

    {
        // NOTE KI update m_spritesSSBO from *index*, not *updateIndex* point
        // - otherwise entries are multiplied, and indexed incorrectly
        for (size_t i = spriteIndex; i < spriteCount; i++) {
            auto& sprite = m_sprites[i];
            sprite.prepare(m_assets);
            for (auto& shape : sprite.m_shapes) {
                m_shapesSSBO.emplace_back(shape.toSSBO());
            }
        }
    }

    const size_t shapeCount = m_shapesSSBO.size();
    {
        constexpr size_t sz = sizeof(ShapeSSBO);

        size_t updateIndex = shapeIndex;

        // NOTE KI *reallocate* SSBO if needed
        if (m_ssbo.m_size < shapeCount * sz) {
            m_ssbo.resizeBuffer(m_shapesSSBO.capacity() * sz);
            updateIndex = 0;
        }

        const size_t updateCount = shapeCount - updateIndex;

        m_ssbo.update(
            updateIndex * sz,
            updateCount * sz,
            &m_shapesSSBO[updateIndex]);
    }

    m_lastSpriteSize = m_sprites.size();
    m_lastShapeSize = m_shapesSSBO.size();
}
