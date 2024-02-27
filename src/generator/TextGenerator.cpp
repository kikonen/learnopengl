#include "TextGenerator.h"

#include <iostream>
#include <array>

#include <fmt/format.h>

#include "model/Node.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "render/Batch.h"

#include "text/TextDraw.h"
#include "text/FontAtlas.h"

#include "render/RenderContext.h"

#include "registry/Registry.h"
#include "registry/NodeSnapshotRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/FontRegistry.h"

#include "mesh/VBO_impl.h"

namespace {
}

TextGenerator::TextGenerator()
    : m_vboAtlasTex{ ATTR_FONT_TEX, VBO_FONT_BINDING, "vbo_font" }
{}

void TextGenerator::prepare(
    const PrepareContext& ctx,
    Node& container)
{
    container.m_instancer = this;
}

void TextGenerator::prepareRT(
    const PrepareContext& ctx,
    Node& container)
{
    m_draw = std::make_unique<text::TextDraw>();
    m_draw->prepareRT(ctx);
    m_vao.prepare("text");

    m_vboAtlasTex.prepareVAO(*m_vao.modifyVAO());
}

void TextGenerator::updateWT(
    const UpdateContext& ctx,
    Node& container)
{
}

void TextGenerator::updateEntity(
    NodeSnapshotRegistry& snapshotRegistry,
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
    if (!m_dirty) return;
    m_dirty = false;

    m_vbo.clear();
    m_vboAtlasTex.clear();

    glm::vec2 pen{ 0.f };

    m_draw->render(
        ctx,
        m_fontId,
        m_text,
        pen,
        m_vbo,
        m_vboAtlasTex);

    m_aabb = m_vbo.calculateAABB();

    m_vbo.m_meshPositionOffset = -m_aabb.getVolume();

    m_vao.clear();
    m_vao.registerModel(m_vbo);
    m_vboAtlasTex.updateVAO(*m_vao.modifyVAO());
    m_vao.updateRT();

    auto* type = container.m_typeHandle.toType();

    auto* lodMesh = type->modifyLod(0);
    auto& lod = lodMesh->m_lod;
    lod.m_baseVertex = m_vbo.getBaseVertex();
    lod.m_baseIndex = m_vbo.getBaseIndex();
    lod.m_indexCount = m_vbo.getIndexCount();
}

const kigl::GLVertexArray* TextGenerator::getVAO(
    const Node& container) const noexcept
{
    return m_vao.getVAO();
}

void TextGenerator::bindBatch(
    const RenderContext& ctx,
    mesh::MeshType* type,
    Node& container,
    render::Batch& batch)
{
    m_draw->updateRT();

    const auto& snapshot = ctx.m_registry->m_activeSnapshotRegistry->getSnapshot(container.m_snapshotIndex);

    batch.addSnapshot(
        ctx,
        type,
        &type->getLod(0)->m_lod,
        snapshot,
        container.m_entityIndex);
}

GLuint64 TextGenerator::getAtlasTextureHandle() const noexcept
{
    return FontRegistry::get().getFont(m_fontId)->getTextureHandle();
}

void TextGenerator::clear()
{
    m_vao.clear();
    m_vbo.clear();
    m_vboAtlasTex.clear();
}
