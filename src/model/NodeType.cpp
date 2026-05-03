#include "NodeType.h"

#include <fmt/format.h>

#include "asset/AABBBuilder.h"

#include "material/CustomMaterial.h"
#include "material/Material.h"

#include "backend/DrawOptions.h"

#include "pool/TypeHandle.h"

#include "mesh/MeshSet.h"
#include "mesh/LodMesh.h"
#include "mesh/LodMeshContainer.h"
#include "mesh/Mesh.h"
#include "mesh/mesh_util.h"

#include "engine/PrepareContext.h"

#include "component/definition/AddonSelectorDefinition.h"
#include "component/definition/CameraComponentDefinition.h"
#include "component/definition/LightDefinition.h"
#include "component/definition/AudioListenerDefinition.h"
#include "component/definition/AudioSourceDefinition.h"
#include "component/definition/TextGeneratorDefinition.h"
#include "component/definition/PhysicsDefinition.h"
#include "component/definition/ControllerDefinition.h"
#include "component/definition/GeneratorDefinition.h"
#include "component/definition/ParticleGeneratorDefinition.h"

#include "material/Material.h"

#include "script/ScriptSystem.h"

#include "registry/NodeRegistry.h"
#include "registry/NodeTypeRegistry.h"

#include "model/Snapshot.h"
#include "model/EntityFlags.h"
#include "model/CompositeDefinition.h"

namespace model
{
    NodeType::NodeType()
        : m_aabb{},
        m_name{ std::make_unique<std::string>() },
        m_scripts{ std::make_unique<std::vector<script::script_id>>() }
    {
    }

    NodeType::NodeType(NodeType&& o) noexcept
        : m_handle{ o.m_handle },
        m_aabb{ o.m_aabb },
        m_name{ std::move(o.m_name) },
        m_flags{ o.m_flags },
        m_layer{ o.m_layer },
        m_preparedWT{ o.m_preparedWT },
        m_preparedRT{ o.m_preparedRT },
        m_meshContainer{ std::move(o.m_meshContainer) },
        m_scripts{ std::move(o.m_scripts) },
        m_customMaterial{ std::move(o.m_customMaterial) },
        m_addonSelectorDefinition{ std::move(o.m_addonSelectorDefinition) },
        m_cameraComponentDefinition{ std::move(o.m_cameraComponentDefinition) },
        m_generatorDefinition{ std::move(o.m_generatorDefinition) },
        m_controllerDefinitions{ std::move(o.m_controllerDefinitions) },
        m_lightDefinition{ std::move(o.m_lightDefinition) },
        m_particleGeneratorDefinition{ std::move(o.m_particleGeneratorDefinition) },
        m_textGeneratorDefinition{ std::move(o.m_textGeneratorDefinition) },
        m_compositeDefinition{ std::move(o.m_compositeDefinition) }
    {
    }

    NodeType::~NodeType()
    {
        KI_INFO(fmt::format("NODE_TYPE: delete - id={}", m_handle.str()));
    }

    std::string NodeType::str() const noexcept
    {
        auto* meshContainer = m_meshContainer.get();

        return fmt::format(
            "<NODE_TYPE: id={}, name={}, meshes={}>",
            m_handle.str(), m_name ? *m_name : "N/A", meshContainer ? meshContainer->size() : 0);
    }

    bool NodeType::hasMeshes() const noexcept
    {
        return m_meshContainer && !m_meshContainer->hasMeshes();
    }

    void NodeType::setMeshContainer(std::unique_ptr<mesh::LodMeshContainer> meshContainer)
    {
        m_meshContainer = std::move(meshContainer);
    }

    const mesh::LodMeshContainer* NodeType::getMeshContainer() const
    {
        return m_meshContainer.get();
    }

    mesh::LodMeshContainer* NodeType::modifyMeshContainer()
    {
        return m_meshContainer.get();
    }

    void NodeType::setAddonMeshContainer(std::unique_ptr<mesh::LodMeshContainer> meshContainer)
    {
        m_addonMeshContainer = std::move(meshContainer);
    }

    const mesh::LodMeshContainer* NodeType::getAddonMeshContainer() const
    {
        return m_addonMeshContainer.get();
    }

    mesh::LodMeshContainer* NodeType::modifyAddonMeshContainer()
    {
        return m_addonMeshContainer.get();
    }

    void NodeType::prepareWT(
        const PrepareContext& ctx)
    {
        if (m_preparedWT) return;
        m_preparedWT = true;

        if (m_particleGeneratorDefinition) {
            m_particleGeneratorDefinition->m_data.m_material->registerMaterial();
        }

        if (m_meshContainer) {
            for (auto& lodMesh : m_meshContainer->modifyLodMeshes()) {
                lodMesh->registerMaterial();
            }
        }

        if (m_addonMeshContainer) {
            for (auto& lodMesh : m_addonMeshContainer->modifyLodMeshes()) {
                lodMesh->registerMaterial();
            }
        }

        if (m_scripts) {
            auto& scriptSystem = script::ScriptSystem::get();

            for (auto scriptId : *m_scripts)
            {
                scriptSystem.bindTypeScript(m_flags.root, m_handle, scriptId);
            }
        }
    }

    void NodeType::prepareRT(
        const PrepareContext& ctx)
    {
        if (m_preparedRT) return;
        m_preparedRT = true;

        if (m_meshContainer) {
            for (auto& lodMesh : m_meshContainer->modifyLodMeshes()) {
                lodMesh->prepareRT(ctx);
            }
        }

        if (m_addonMeshContainer) {
            for (auto& lodMesh : m_addonMeshContainer->modifyLodMeshes()) {
                lodMesh->prepareRT(ctx);
            }
        }

        if (m_customMaterial) {
            m_customMaterial->prepareRT(ctx);
            NodeTypeRegistry::get().registerCustomMaterial(m_handle);
        }
    }

    void NodeType::bind(const render::RenderContext& ctx)
    {
        assert(isReady());
    }

    void NodeType::setCustomMaterial(std::unique_ptr<CustomMaterial> customMaterial) noexcept
    {
        m_customMaterial = std::move(customMaterial);
    }

    ki::size_t_entity_flags NodeType::resolveEntityFlags() const noexcept {
        ki::size_t_entity_flags flags = 0;

        if (m_flags.noFrustum) {
            flags |= model::ENTITY_NO_FRUSTUM_BIT;
        }
        return flags;
    }

    void NodeType::prepareVolume() noexcept {
        m_aabb = calculateAABB();
    }

    AABB NodeType::calculateAABB() const noexcept
    {
        if (!m_meshContainer) return {};

		AABBBuilder builder;

        for (auto& lodMesh : m_meshContainer->getLodMeshes()) {
            if (lodMesh->m_flags.noVolume) continue;
            builder.minmax(lodMesh->calculateAABB());
        }

        return builder.toAABB();
    }
}
