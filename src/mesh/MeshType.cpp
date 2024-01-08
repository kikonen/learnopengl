#include "MeshType.h"

#include <fmt/format.h>

#include "asset/Program.h"
#include "asset/CustomMaterial.h"
#include "asset/Material.h"
#include "asset/Sprite.h"

#include "backend/DrawOptions.h"

#include "mesh/Mesh.h"

#include "engine/PrepareContext.h"

#include "registry/NodeRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/SpriteRegistry.h"


class Assets;

namespace {
}

namespace mesh {
    MeshType::MeshType(std::string_view name)
        : m_name{ name },
        m_materialVBO{ std::make_unique<MaterialVBO>()}
    {
    }

    MeshType::MeshType(MeshType&& o) noexcept
        : m_id{ o.m_id },
        m_name{ o.m_name },
        m_entityType{ o.m_entityType },
        m_flags{ o.m_flags },
        m_priority{ o.m_priority },
        m_program{ o.m_program },
        m_shadowProgram{ o.m_shadowProgram },
        m_preDepthProgram{ o.m_preDepthProgram },
        m_materialVBO{ std::move(o.m_materialVBO) },
        m_sprite{ std::move(o.m_sprite) },
        m_materialIndex{ o.m_materialIndex },
        m_drawOptions{ o.m_drawOptions },
        m_vao{ o.m_vao },
        m_prepared{ o.m_prepared },
        m_mesh{ o.m_mesh },
        m_deleter{ std::move(o.m_deleter) },
        m_customMaterial{ std::move(o.m_customMaterial) },
        m_privateVAO{ o.m_privateVAO }
    {
    }

    MeshType::~MeshType()
    {
        KI_INFO(fmt::format("NODE_TYPE: delete iD={}", m_id));
    }

    const std::string MeshType::str() const noexcept
    {
        return fmt::format(
            "<NODE_TYPE: id={}, name={}, mesh={}, vao={}, materialIndex={}, materialCount={}>",
            m_id, m_name, m_mesh ? m_mesh->str() : "N/A", m_vao ? *m_vao : -1,
            m_materialIndex,
            m_materialVBO->getMaterialCount());
    }

    void MeshType::setMesh(std::unique_ptr<Mesh> mesh, bool umique)
    {
        setMesh(mesh.get());
        m_deleter = std::move(mesh);
    }

    void MeshType::setMesh(Mesh* mesh)
    {
        m_mesh = mesh;
        if (!m_mesh) return;

        m_materialVBO->setMaterials(m_mesh->getMaterials());
    }

    void MeshType::modifyMaterials(std::function<void(Material&)> fn)
    {
        for (auto& material : m_materialVBO->modifyMaterials()) {
            fn(material);
        }
    }

    void MeshType::prepare(
        const PrepareContext& ctx)
    {
        if (!m_mesh) return;

        if (m_prepared) return;
        m_prepared = true;

        for (auto& material : m_materialVBO->modifyMaterials()) {
            ctx.m_registry->m_materialRegistry->registerMaterial(material);
        }

        if (m_entityType == EntityType::sprite && m_sprite) {
            ctx.m_registry->m_spriteRegistry->registerSprite(*m_sprite);
        }

        m_vao = m_mesh->prepareRT(ctx);

        {
            m_mesh->prepareMaterials(*m_materialVBO);

            ctx.m_registry->m_materialRegistry->registerMaterialVBO(*m_materialVBO);
            m_materialIndex = m_materialVBO->resolveMaterialIndex();
        }

        {
            m_drawOptions.m_renderBack = m_flags.renderBack;
            m_drawOptions.m_wireframe = m_flags.wireframe;
            m_drawOptions.m_blend = m_flags.blend;
            m_drawOptions.m_blendOIT = m_flags.blendOIT;
            m_drawOptions.m_instanced = m_flags.instanced;
            m_drawOptions.m_tessellation = m_flags.tessellation;

            m_mesh->prepareDrawOptions(m_drawOptions);
        }
    }

    void MeshType::prepareRT(
        const PrepareContext& ctx)
    {
        if (!m_mesh) return;

        if (m_preparedView) return;
        m_preparedView = true;

        //m_privateVAO.create();

        m_vao = m_mesh->prepareRT(ctx);

        if (m_program) {
            m_program->prepareRT(ctx.m_assets);
        }

        if (m_shadowProgram) {
            m_shadowProgram->prepareRT(ctx.m_assets);
        }

        if (m_preDepthProgram) {
            m_preDepthProgram->prepareRT(ctx.m_assets);
        }

        if (m_customMaterial) {
            m_customMaterial->prepareRT(ctx);
        }
    }

    void MeshType::bind(const RenderContext& ctx)
    {
        assert(isReady());

        if (m_customMaterial) {
            m_customMaterial->bindTextures(ctx);
        }
    }

    void MeshType::setCustomMaterial(std::unique_ptr<CustomMaterial> customMaterial) noexcept
    {
        m_customMaterial = std::move(customMaterial);
    }
}
