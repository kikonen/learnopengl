#include "TextGenerator.h"

#include "model/Node.h"

#include "mesh/MeshType.h"

TextGenerator::TextGenerator()
{}

void TextGenerator::prepare(
    const Assets& assets,
    Registry* registry,
    Node& container)
{}

void TextGenerator::update(
    const UpdateContext& ctx,
    Node& container)
{}

void TextGenerator::updateVAO(
    const RenderContext& ctx,
    const Node& container)
{}

const kigl::GLVertexArray* TextGenerator::getVAO(
    const Node& container) const noexcept
{
    return container.m_type->getVAO();
}

const backend::DrawOptions& TextGenerator::getDrawOptions(
    const Node& container) const noexcept
{
    return container.m_type->getDrawOptions();
}
