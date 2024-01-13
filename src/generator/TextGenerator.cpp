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

namespace {
    std::array<std::string,6> texts{
        "This the story",
        "And it will continue",
        "So be prepared",
        "until the end",
        "Viva la vida!",
//        "Terveisi\u00e4 T\u00e4\u00e4lt\u00e4 - \u00c4\u00c5\u00d6 \u00e4\u00e5\u00f6",
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
    //std::cout << fmt::format("total={}, elapsed={}\n", elapsed, ctx.m_clock.elapsedSecs);

    elapsed += ctx.m_clock.elapsedSecs;

    constexpr float step = 20.f;
    bool hit = elapsed >= step;

    if (hit) {
        elapsed -= step;
        setText(texts[index++]);
        index = index % texts.size();
        m_dirty = true;
    }

    if (!m_dirty) return;
    m_dirty = false;

    m_draw->prepareRT(ctx.toPrepareContext());

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
    m_draw->updateRT(ctx.m_state);

    const auto& snapshot = ctx.m_registry->m_snapshotRegistry->getActiveSnapshot(container.m_snapshotIndex);
    batch.addSnapshot(ctx, snapshot);
}
