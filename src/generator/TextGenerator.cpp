#include "TextGenerator.h"

#include <iostream>
#include <array>

#include <fmt/format.h>

#include "model/Node.h"
#include "model/Snapshot.h"

#include "mesh/LodMesh.h"
#include "mesh/TextMesh.h"
#include "mesh/Transform.h"

#include "mesh/vao/VBO_impl.h"

#include "render/Batch.h"

#include "render/RenderContext.h"

#include "registry/Registry.h"
#include "registry/EntityRegistry.h"

#include "text/FontRegistry.h"
#include "text/FontAtlas.h"

#include "text/Align.h"
#include "text/TextDraw.h"
#include "text/TextSystem.h"
#include "text/vao/TextVAO.h"


TextGenerator::TextGenerator()
{}

TextGenerator::~TextGenerator() = default;

void TextGenerator::prepareWT(
    const PrepareContext& ctx,
    Node& container)
{
}

void TextGenerator::prepareRT(
    const PrepareContext& ctx,
    Node& container)
{
    m_mesh = std::make_shared<mesh::TextMesh>();
    m_mesh->prepareVAO();

    {
        auto& lodMesh = m_lodMeshes.emplace_back();
        lodMesh.setMesh(m_mesh);

        auto* src = container.modifyLodMesh(0);
        lodMesh.setMaterial(src->getMaterial());
        lodMesh.m_drawOptions = src->m_drawOptions;
        lodMesh.prepareRT(ctx);
    }

    m_draw = std::make_unique<text::TextDraw>();
    m_draw->prepareRT(ctx);
}

void TextGenerator::updateWT(
    const UpdateContext& ctx,
    const Node& container)
{
}

//void TextGenerator::updateEntity(
//    EntityRegistry& entityRegistry,
//    Node& container)
//{
//    auto& state = container.modifyState();
//    auto& snapshot = snapshotRegistry.modifySnapshot(container.m_snapshotIndex);
//    auto* entity = entityRegistry.modifyEntity(container.m_entityIndex, true);
//
//    //const glm::vec4 volume{ 0.f, 0.f, 0.f, m_aabb.getVolume().w };
//    const glm::vec4& volume = m_aabb.getVolume();
//
//    state.setVolume(volume);
//    snapshot.setVolume(volume);
//    entity->u_volume = volume;
//}

void TextGenerator::updateVAO(
    const RenderContext& ctx,
    const Node& container)
{
    if (!m_dirty) return;
    m_dirty = false;

    auto& lodMesh = m_lodMeshes[0];
    auto* mesh = lodMesh.getMesh<mesh::TextMesh>();

    mesh->clear();

    m_draw->render(
        m_fontId,
        m_text,
        m_pivot,
        m_alignHorizontal,
        m_alignVertical,
        mesh);

    m_aabb = mesh->calculateAABB(glm::mat4{1.f});

    text::TextVAO* vao = text::TextSystem::get().getTextVAO();

    vao->updateVertices(
        mesh->m_vboIndex,
        mesh->m_vertices);

    vao->updateIndeces(
        mesh->m_eboIndex,
        mesh->m_indeces);

    vao->updateAtlasCoords(
        mesh->m_vboIndex,
        mesh->m_atlasCoords);

    lodMesh.m_indexCount = mesh->getIndexCount();

    // TODO KI threading violation
    container.modifyState().setVolume(m_aabb.getVolume());

    //{
    //    m_vao.m_positionVbo.m_positionOffset = m_aabb.getVolume();
    //    mesh->m_positionVboOffset = m_vao.m_positionVbo.addEntries(mesh->m_positions);
    //    mesh->m_indexEboOffset = m_vao.m_indexEbo.addIndeces(mesh->m_indeces);

    //    m_vao.m_normalVbo.addEntries(mesh->m_normals);
    //    m_vao.m_textureVbo.addEntries(mesh->m_texCoords);

    //    m_vboAtlasTex.addEntries(mesh->m_atlasCoords);
    //    m_vboAtlasTex.updateVAO(*m_vao.modifyVAO());

    //    m_vao.updateRT();
    //}
}

void TextGenerator::bindBatch(
    const RenderContext& ctx,
    const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
    const std::function<void(ki::program_id)>& programPrepare,
    uint8_t kindBits,
    render::Batch& batch,
    const Node& container,
    const Snapshot& snapshot)
{
    m_draw->updateRT();

    batch.addSnapshot(
        ctx,
        &container,
        programSelector,
        programPrepare,
        kindBits,
        snapshot,
        container.m_entityIndex);
}

GLuint64 TextGenerator::getAtlasTextureHandle() const noexcept
{
    return text::FontRegistry::get().getFont(m_fontId)->getTextureHandle();
}

void TextGenerator::clear()
{
}
