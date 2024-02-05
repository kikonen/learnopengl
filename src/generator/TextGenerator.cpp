#include "TextGenerator.h"

#include <iostream>
#include <array>

#include <fmt/format.h>

#include "model/Node.h"

#include "mesh/MeshType.h"

#include "render/Batch.h"

#include "text/TextDraw.h"

#include "render/RenderContext.h"

#include "registry/Registry.h"
#include "registry/SnapshotRegistry.h"
#include "registry/EntityRegistry.h"

namespace {
    std::array<std::string,3> texts{
R"(This the story
And it will continue
So be prepared
until the end)",
"Viva la vida!",
// "Terveisi\u00e4 T\u00e4\u00e4lt\u00e4 - \u00c4\u00c5\u00d6 \u00e4\u00e5\u00f6",
"Terveisiä täältä! - ÄÅÖ - äåö",
    };

    float elapsed = 0.f;
    int index = 0;
}

TextGenerator::TextGenerator()
{}

void TextGenerator::prepare(
    const PrepareContext& ctx,
    Node& container)
{
    auto* type = container.m_typeHandle.toType();
    m_drawOptions = type->getDrawOptions();

    container.m_instancer = this;
}

void TextGenerator::prepareRT(
    const PrepareContext& ctx,
    Node& container)
{
    m_draw = std::make_unique<text::TextDraw>();
    m_draw->prepareRT(ctx);
    m_vao.prepare("text");
}

void TextGenerator::updateWT(
    const UpdateContext& ctx,
    Node& container)
{
}

void TextGenerator::updateEntity(
    SnapshotRegistry& snapshotRegistry,
    EntityRegistry& entityRegistry,
    Node& container)
{
    auto& transform = container.modifyTransform();
    auto& snapshot = snapshotRegistry.modifySnapshot(container.m_snapshotIndex);
    auto* entity = entityRegistry.modifyEntity(container.m_entityIndex, true);

    const glm::vec4 volume{ 0.f, 0.f, 0.f, m_aabb.getVolume().w };

    transform.setVolume(volume);
    snapshot.m_volume = volume;
    entity->u_volume = volume;
}

void TextGenerator::updateVAO(
    const RenderContext& ctx,
    const Node& container)
{
    //std::cout << fmt::format("total={}, elapsed={}\n", elapsed, ctx.m_clock.elapsedSecs);

    elapsed += ctx.m_clock.elapsedSecs;

    constexpr float step = 20.f;
    bool hit = elapsed >= step;
    //hit = false;
    if (hit) {
        elapsed -= step;
        setText(texts[index++]);
        index = index % texts.size();
        m_dirty = true;
    }

    if (!m_dirty) return;
    m_dirty = false;

    m_vbo.clear();

    glm::vec2 pen{ 0.f };

    m_draw->render(
        ctx,
        m_fontId,
        m_text,
        pen,
        m_vbo);

    m_aabb = m_vbo.calculateAABB();

    m_vbo.m_positionOffset = -m_aabb.getVolume();

    m_vao.clear();
    m_vao.registerModel(m_vbo);
    m_vao.updateRT();

    m_drawOptions.m_vertexOffset = static_cast<uint32_t>(m_vbo.m_vertexOffset);
    m_drawOptions.m_indexOffset = static_cast<uint32_t>(m_vbo.m_indexOffset);
    m_drawOptions.m_indexCount = static_cast<uint32_t>(m_vbo.getIndexCount());
}

const kigl::GLVertexArray* TextGenerator::getVAO(
    const Node& container) const noexcept
{
    return m_vao.getVAO();
}

const backend::DrawOptions& TextGenerator::getDrawOptions(
    const Node& container) const noexcept
{
    return m_drawOptions;
}

void TextGenerator::bindBatch(
    const RenderContext& ctx,
    Node& container,
    render::Batch& batch)
{
    m_draw->updateRT();

    auto& registry = Registry::get();
    auto& nr = *registry.m_snapshotRegistry;

    const auto& snapshot = nr.getActiveSnapshot(container.m_snapshotIndex);
    batch.addSnapshot(ctx, snapshot, container.m_entityIndex);
}

void TextGenerator::clear()
{
    m_vao.clear();
    m_vbo.clear();
}
