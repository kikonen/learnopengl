#include "TextGenerator.h"

#include "model/Node.h"

#include "mesh/MeshType.h"

#include "render/Batch.h"

#include "text/TextDraw.h"

#include "render/RenderContext.h"


TextGenerator::TextGenerator()
{}

void TextGenerator::prepare(
    const Assets& assets,
    Registry* registry,
    Node& container)
{
    m_draw = std::make_unique<text::TextDraw>();

    m_drawOptions.type = backend::DrawOptions::Type::elements;
    m_drawOptions.mode = GL_TRIANGLES;

    container.m_instancer = this;
}

void TextGenerator::update(
    const UpdateContext& ctx,
    Node& container)
{}

void TextGenerator::updateVAO(
    const RenderContext& ctx,
    const Node& container)
{
    if (!m_dirty) return;
    m_dirty = false;

    m_draw->prepareRT(ctx.m_assets, ctx.m_registry);

    m_draw->draw(
        ctx,
        m_fontId,
        m_text,
        m_drawOptions,
        &container);
}

const kigl::GLVertexArray* TextGenerator::getVAO(
    const Node& container) const noexcept
{
    return m_draw->getVAO();
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
    batch.add(ctx, container.getSnapshot().m_entityIndex);
}
