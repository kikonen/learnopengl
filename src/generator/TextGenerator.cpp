#include "TextGenerator.h"

#include <iostream>
#include <array>

#include <fmt/format.h>

#include "util/thread.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "mesh/LodMesh.h"
#include "mesh/TextMesh.h"
#include "mesh/Transform.h"

#include "mesh/vao/VBO_impl.h"

#include "render/Batch.h"
#include "render/InstanceRegistry.h"
#include "render/DrawableInfo.h"
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
    : m_draw{ std::make_unique<text::TextDraw>() }
{
    m_updateDrawables = true;
}

TextGenerator::~TextGenerator() = default;

void TextGenerator::setMesh(const util::Ref<mesh::TextMesh> mesh)
{
    m_mesh = mesh;
}

void TextGenerator::updateRT(
    const UpdateContext& ctx,
    const model::Node& container)
{
    if (!m_dirty) return;
    m_dirty = false;

    mesh::TextMesh* mesh = m_mesh.get();

    mesh->clear();

    // TODO KI race condition between WT and RT
    std::string text = m_text;

    m_draw->render(
        m_fontId,
        text,
        m_pivot,
        m_alignHorizontal,
        m_alignVertical,
        mesh);

    const auto& aabb = mesh->calculateAABB(glm::mat4{1.f});

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

    SphereVolume worldVolume;
    {
        const auto* snapshot = container.getSnapshotRT();
        const auto& localVolume = aabb.toLocalVolume();
        worldVolume = localVolume.calculateWorldVolume(
            snapshot->getModelMatrix(),
            snapshot->getMaxScale());
    }

    {
        const auto& ref = container.m_instanceRef;
        auto& instanceRegistry = render::InstanceRegistry::get();
        auto drawables = instanceRegistry.modifyRange(ref);
        auto& drawable = drawables[0];
        {
            drawable.indexCount = mesh->getIndexCount();
            drawable.worldVolume = worldVolume;
        }
        instanceRegistry.markDirty(ref);
        instanceRegistry.updateInstances(ref);
        instanceRegistry.upload(ref);
    }
}

void TextGenerator::updateDrawables(
    render::InstanceRegistry& instanceRegistry,
    const model::Node& container,
    const model::Snapshot& snapshot)
{
}

void TextGenerator::addToBatch(
    const render::RenderContext& ctx,
    const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
    const std::function<void(ki::program_id)>& programPrepare,
    uint8_t kindBits,
    render::Batch& batch,
    const model::Node& container)
{
    batch.addDrawablesSingleNode(
        ctx,
        container.getType(),
        container.m_instanceRef,
        programSelector,
        programPrepare,
        kindBits);
}

GLuint64 TextGenerator::getAtlasTextureHandle() const noexcept
{
    auto* fontAtlas = text::FontRegistry::get().getFontAtlas(m_fontId);
    if (!fontAtlas) {
        fontAtlas = text::FontRegistry::get().getDefaultFontAtlas();
    }

    return fontAtlas ? fontAtlas->getTextureHandle() : 0;
}

void TextGenerator::clear()
{
}
