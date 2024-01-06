#include "TextGenerator.h"

#include <array>

#include "model/Node.h"

#include "mesh/MeshType.h"

#include "render/Batch.h"

#include "text/TextDraw.h"

#include "render/RenderContext.h"

namespace {
    std::array<std::string,6> texts{
        "This the story",
        "And it will continue",
        "So be prepared",
        "until the end",
        "Viva la vida!",
        "Terveisi‰ T‰‰lt‰. ƒ≈÷ ‰Âˆ",
    };

    float elapsed = 0.f;
    int index = 0;
}

TextGenerator::TextGenerator()
{}

void TextGenerator::prepare(
    const Assets& assets,
    Registry* registry,
    Node& container)
{
    m_draw = std::make_unique<text::TextDraw>();

    m_drawOptions = container.m_type->getDrawOptions();

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
    elapsed += ctx.m_clock.elapsedSecs;

    bool hit = elapsed >= 20.f;

    if (hit) {
        elapsed -= 20.f;
        setText(texts[index++]);
        index = index % texts.size();
        m_dirty = true;
    }

    if (!m_dirty) return;
    m_dirty = false;

    m_draw->prepareRT(ctx.m_assets, ctx.m_registry);

    m_draw->render(
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
