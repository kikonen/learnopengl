#include "TextGenerator.h"

#include <iostream>
#include <array>

#include <fmt/format.h>

#include "model/Node.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"
#include "mesh/TextMesh.h"
#include "mesh/VBO_impl.h"

#include "render/Batch.h"

#include "render/RenderContext.h"

#include "registry/Registry.h"
#include "registry/NodeSnapshotRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/FontRegistry.h"


#include "text/TextDraw.h"
#include "text/FontAtlas.h"

namespace {
}

TextGenerator::TextGenerator()
    : m_vboAtlasTex{ "vbo_font", ATTR_FONT_ATLAS_TEX, VBO_FONT_ATLAS_BINDING }
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

    auto* type = container.m_typeHandle.toType();

    auto* lodMesh = type->modifyLod(0);
    auto& lod = lodMesh->m_lod;
    auto* mesh = lodMesh->getMesh<mesh::TextMesh>();

    mesh->clear();

    m_vboAtlasTex.clear();

    glm::vec2 pen{ 0.f };

    m_draw->render(
        ctx,
        m_fontId,
        m_text,
        pen,
        mesh);

    m_aabb = mesh->calculateAABB();

    m_vao.clear();

    {
        m_vao.m_positionVbo.m_positionOffset = m_aabb.getVolume();
        mesh->m_positionVboOffset = m_vao.m_positionVbo.addEntries(mesh->m_positions);
        mesh->m_indexEboOffset = m_vao.m_indexEbo.addIndeces(mesh->m_indeces);

        m_vao.m_normalVbo.addEntries(mesh->m_normals);
        m_vao.m_textureVbo.addEntries(mesh->m_texCoords);

        m_vboAtlasTex.addEntries(mesh->m_atlasCoords);
        m_vboAtlasTex.updateVAO(*m_vao.modifyVAO());

        m_vao.updateRT();
    }

    lod.m_baseVertex = mesh->getBaseVertex();
    lod.m_baseIndex = mesh->getBaseIndex();
    lod.m_indexCount = mesh->getIndexCount();
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
    //auto* mesh = m_mesh.get();
    //mesh->clear();
    //m_vboAtlasTex.clear();
    //m_vao.clear();
}
