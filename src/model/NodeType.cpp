#include "NodeType.h"

#include <fmt/format.h>

#include "material/CustomMaterial.h"
#include "material/Material.h"

#include "backend/DrawOptions.h"

#include "pool/TypeHandle.h"

#include "mesh/MeshSet.h"
#include "mesh/LodMesh.h"
#include "mesh/Mesh.h"
#include "mesh/mesh_util.h"

#include "engine/PrepareContext.h"

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
#include "registry/ModelRegistry.h"
#include "registry/NodeTypeRegistry.h"

#include "model/Snapshot.h"
#include "model/EntityFlags.h"
#include "model/CompositeDefinition.h"

NodeType::NodeType()
    : m_aabb{ std::make_unique<AABB>() },
    m_name{ std::make_unique<std::string>() },
    m_scripts{ std::make_unique<std::vector<script::script_id>>() }
{}

NodeType::NodeType(NodeType&& o) noexcept
    : m_handle{ o.m_handle },
    m_aabb{ std::move(o.m_aabb) },
    m_name{ std::move(o.m_name) },
    m_flags{ o.m_flags },
    m_layer{ o.m_layer },
    m_preparedWT{ o.m_preparedWT },
    m_preparedRT{ o.m_preparedRT },
    m_lodMeshes{ std::move(o.m_lodMeshes) },
    m_scripts{ std::move(o.m_scripts) },
    m_customMaterial{ std::move(o.m_customMaterial) },
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
    auto* lodMesh = getLodMesh(0);

    return fmt::format(
        "<NODE_TYPE: id={}, name={}, lod={}>",
        m_handle.str(), *m_name, lodMesh ? lodMesh->str() : "N/A");
}

uint16_t NodeType::addMeshSet(
    const mesh::MeshSet& meshSet)
{
    uint16_t count = 0;

    for (auto& mesh : meshSet.getMeshes()) {
        auto* lodMesh = addLodMesh({ mesh });
        count++;
    }

    return count;
}

mesh::LodMesh* NodeType::addLodMesh(
    mesh::LodMesh&& lodmesh)
{
    m_lodMeshes.push_back(std::move(lodmesh));
    return &m_lodMeshes[m_lodMeshes.size() - 1];
}

void NodeType::prepareWT(
    const PrepareContext& ctx)
{
    if (m_preparedWT) return;
    m_preparedWT = true;

    if (m_particleGeneratorDefinition) {
        m_particleGeneratorDefinition->m_data.m_material->registerMaterial();
    }

    for (auto& lodMesh : m_lodMeshes) {
        lodMesh.registerMaterial();
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

    for (auto& lodMesh : m_lodMeshes) {
        lodMesh.prepareRT(ctx);
    }

    if (m_customMaterial) {
        m_customMaterial->prepareRT(ctx);
        NodeTypeRegistry::get().registerCustomMaterial(m_handle);
    }
}

void NodeType::bind(const RenderContext& ctx)
{
    assert(isReady());
}

std::shared_ptr<animation::RigContainer> NodeType::findRig() const
{
    return mesh::findRig(m_lodMeshes);
}

void NodeType::setCustomMaterial(std::unique_ptr<CustomMaterial> customMaterial) noexcept
{
    m_customMaterial = std::move(customMaterial);
}

ki::size_t_entity_flags NodeType::resolveEntityFlags() const noexcept {
    ki::size_t_entity_flags flags = 0;

    if (m_flags.noFrustum) {
        flags |= ENTITY_NO_FRUSTUM_BIT;
    }
    return flags;
}

void NodeType::prepareVolume() noexcept {
    *m_aabb = calculateAABB();
}

AABB NodeType::calculateAABB() const noexcept
{
    if (m_lodMeshes.empty()) return {};

    AABB aabb{ true };

    for (auto& lodMesh : m_lodMeshes) {
        if (lodMesh.m_flags.noVolume) continue;
        aabb.minmax(lodMesh.calculateAABB());
    }

    aabb.updateVolume();

    return aabb;
}
