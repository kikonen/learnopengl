#include "TextGenerator.h"

#include <iostream>
#include <array>

#include <fmt/format.h>

#include "util/thread.h"

#include "model/Node.h"
#include "model/Snapshot.h"
#include "model/NodeType.h"

#include "mesh/LodMeshContainer.h"
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

#include "material/MaterialRegistry.h"
#include "material/Material.h"

#include "text/FontRegistry.h"
#include "text/FontAtlas.h"

#include "text/Align.h"
#include "text/TextDraw.h"
#include "text/TextSystem.h"
#include "text/vao/TextVAO.h"


TextGenerator::TextGenerator()
    : m_draw{ std::make_unique<text::TextDraw>() }
{
    m_lightWeight = true;
    m_updateDrawables = true;
}

TextGenerator::~TextGenerator() = default;

void TextGenerator::prepareWT(
    const PrepareContext& ctx,
    model::Node& container)
{
    ASSERT_WT();
}

void TextGenerator::prepareRT(
    const PrepareContext& ctx,
    model::Node& container,
    const model::Snapshot& snapshot)
{
    ASSERT_RT();

    // NOTE KI material and such are defined in template
    auto* templateLodMesh = container.getType()->getMeshContainer()->getLodMesh(0);

    auto mesh = util::Ref<mesh::TextMesh>::create();
    mesh->setMaterial(templateLodMesh->m_material);
    mesh->m_maxSize = m_maxSize;

    auto lodMesh = util::Ref<mesh::LodMesh>::create();
    lodMesh->setMesh(mesh);
    lodMesh->registerMaterial();

    {
        lodMesh->m_minDistance2 = templateLodMesh->m_minDistance2;
        lodMesh->m_maxDistance2 = templateLodMesh->m_maxDistance2;

        lodMesh->m_drawOptions = templateLodMesh->m_drawOptions;

        //lodMesh->m_programId = templateLodMesh->m_programId;
        //lodMesh->m_oitProgramId = templateLodMesh->m_oitProgramId;
        //lodMesh->m_shadowProgramId = templateLodMesh->m_shadowProgramId;
        //lodMesh->m_preDepthProgramId = templateLodMesh->m_preDepthProgramId;
        //lodMesh->m_selectionProgramId = templateLodMesh->m_selectionProgramId;
        //lodMesh->m_idProgramId = templateLodMesh->m_idProgramId;
        //lodMesh->m_normalProgramId = templateLodMesh->m_normalProgramId;

        lodMesh->m_baseTransform = templateLodMesh->m_baseTransform;
        lodMesh->m_flags = templateLodMesh->m_flags;
    }

    lodMesh->prepareRT(ctx);

    m_mesh = mesh;
    m_lodMesh = lodMesh;
}

void TextGenerator::registerDrawables(
    render::InstanceRegistry& instanceRegistry,
    const model::Node& container,
    const model::Snapshot& snapshot)
{
    auto entityIndex = container.getEntityIndex();
    uint32_t groupId = 0;

    m_instanceRef = instanceRegistry.allocate(1);
    
    const auto& lodMesh = *m_lodMesh;
    auto& drawable = instanceRegistry.modifyRange(m_instanceRef)[0];
    {
        drawable.meshId = m_mesh->getId();
        drawable.groupId = groupId;

        drawable.entityIndex = entityIndex;
        drawable.materialIndex = lodMesh.getMaterialIndex();
        drawable.jointBaseIndex = 0;

        drawable.baseVertex = lodMesh.getBaseVertex();
        drawable.baseIndex = lodMesh.getBaseIndex();
        drawable.indexCount = lodMesh.getIndexCount();

        drawable.minDistance2 = lodMesh.m_minDistance2;
        drawable.maxDistance2 = lodMesh.m_maxDistance2;

        drawable.data = 0;

        drawable.vaoId = lodMesh.m_vaoId;
        drawable.drawOptions = lodMesh.m_drawOptions;

        drawable.programId = lodMesh.m_programId;
        drawable.oitProgramId = lodMesh.m_oitProgramId;
        drawable.shadowProgramId = lodMesh.m_shadowProgramId;
        drawable.preDepthProgramId = lodMesh.m_preDepthProgramId;
        drawable.selectionProgramId = lodMesh.m_selectionProgramId;
        drawable.idProgramId = lodMesh.m_idProgramId;
        drawable.normalProgramId = lodMesh.m_normalProgramId;

        // TODO KI volume/transform can change per frame
        drawable.worldVolume = SphereVolume{ 1, 1, 1, 1 };
        drawable.localTransform = lodMesh.m_baseTransform;

        drawable.m_ignoredBy = container.m_ignoredBy;
        drawable.m_flags = render::toDrawableFlags(container.m_typeFlags, lodMesh.m_flags.noShadow);
        drawable.m_flags.hidden = !container.m_visible;
    }
}

void TextGenerator::updateRT(
    const UpdateContext& ctx,
    const model::Node& container)
{
    if (!m_fontRegistered) {
        updateMaterial(container);
    }

    if (!m_dirty) return;

    auto* fontAtlas = text::FontRegistry::get().getPreparedFontAtlas(m_fontId, true);
    if (!fontAtlas) return;

    m_dirty = false;

    mesh::TextMesh* mesh = m_mesh.get();

    mesh->clear();

    // TODO KI race condition between WT and RT
    std::string text = m_text;

    m_draw->render(
        fontAtlas,
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
        const auto& ref = m_instanceRef;
        auto& instanceRegistry = render::InstanceRegistry::get();
        auto drawables = instanceRegistry.modifyRange(ref);
        auto& drawable = drawables[0];
        {
            drawable.indexCount = mesh->getIndexCount();
            drawable.worldVolume = worldVolume;
            drawable.m_ignoredBy = container.m_ignoredBy;
            drawable.m_flags = render::toDrawableFlags(container.m_typeFlags, false);
            drawable.m_flags.hidden = !container.m_visible;
        }
        instanceRegistry.markDirty(ref);
        instanceRegistry.updateInstances(ref);
        instanceRegistry.upload(ref);
    }
}

void TextGenerator::updateMaterial(const model::Node& container)
{
    auto* fontAtlas = text::FontRegistry::get().getPreparedFontAtlas(m_fontId, true);
    if (!fontAtlas) return;

    auto* lodMesh = m_lodMesh.get();
    auto* material = lodMesh->m_material.get();

    if (material) {
        if (material->m_registeredIndex <= 0) return;

        auto atlasTex = getAtlasTextureHandle();
        if (material->m_fontAtlasTex != atlasTex) {
            material->m_fontAtlasTex = atlasTex;
            MaterialRegistry::get().updateMaterial(material);
            m_fontRegistered = true;
        }
    }
}

void TextGenerator::updateDrawables(
    render::InstanceRegistry& instanceRegistry,
    const model::Node& container,
    const model::Snapshot& snapshot)
{
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
