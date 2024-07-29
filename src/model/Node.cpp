#include "Node.h"

#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "asset/Assets.h"

#include "kigl/kigl.h"

#include "pool/NodeHandle.h"

#include "component/Light.h"
#include "component/CameraComponent.h"

#include "particle/ParticleGenerator.h"

#include "generator/NodeGenerator.h"

#include "animation/RigContainer.h"
#include "animation/AnimationSystem.h"

#include "mesh/mesh_util.h"

#include "mesh/LodMesh.h"
#include "mesh/ModelMesh.h"
#include "mesh/MeshType.h"

#include "model/EntityFlags.h"


#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/NodeSnapshotRegistry.h"
#include "registry/EntityRegistry.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

#include "render/Batch.h"

namespace {
    const static glm::mat4 IDENTITY_MATRIX{ 1.f };
}

Node::Node()
{
}

Node::Node(Node&& o) noexcept
    : m_handle{ o.m_handle }
{}

Node& Node::operator=(Node&& o) noexcept
{
    if (&o == this) return *this;
    return *this;
}


Node::~Node()
{
    KI_INFO(fmt::format("NODE: delete - {}", str()));
}

std::string Node::str() const noexcept
{
    auto* type = m_typeHandle.toType();
    return fmt::format(
        "<NODE: id={}, type={}>",
        m_handle.str(), type ? type->str() : "<null>");
}

void Node::prepareWT(
    const PrepareContext& ctx)
{
    auto& registry = ctx.m_registry;

    auto* type = m_typeHandle.toType();

    if (type->hasMesh()) {
        KI_DEBUG(fmt::format("ADD_ENTITY: {}", str()));

        {
            m_state.m_flags = type->resolveEntityFlags();
        }
    }

    {
        auto& snapshotRegistry = *ctx.m_registry->m_workerSnapshotRegistry;
        m_snapshotIndex = snapshotRegistry.registerSnapshot();
        auto& snapshot = snapshotRegistry.modifySnapshot(m_snapshotIndex);
        snapshot.m_handle = toHandle();
    }

    {
        // NOTE KI for now, allow only single Rig per mesh type
        // i.e. not possible to attach animated attachments
        // or have separate animations for LOD level meshes
        const auto rig = mesh::findRig(type->modifyLodMeshes());

        if (rig) {
            auto [boneBaseIndex, socketBaseIndex] = animation::AnimationSystem::get().registerInstance(*rig);
            m_state.m_boneBaseIndex = boneBaseIndex;
            m_state.m_socketBaseIndex = socketBaseIndex;
            //m_state.m_animationClipIndex = ;
        }
    }

    if (m_generator) {
        m_generator->prepare(ctx, *this);
    }

    if (m_particleGenerator) {
        m_particleGenerator->prepareWT();
    }
}

void Node::prepareRT(
    const PrepareContext& ctx)
{
    auto* type = m_typeHandle.toType();

    const auto& snapshot = ctx.m_registry->m_activeSnapshotRegistry->getSnapshot(m_snapshotIndex);
    if (m_camera) {
        m_camera->snapToIdeal(snapshot);
    }
}

bool Node::isEntity() const noexcept
{
    auto* type = m_typeHandle.toType();
    return type->hasMesh() &&
        !type->m_flags.invisible;
}

void Node::updateVAO(const RenderContext& ctx) noexcept
{
    if (m_instancer) {
        m_instancer->updateVAO(ctx, *this);
    }
}

void Node::bindBatch(
    const RenderContext& ctx,
    mesh::MeshType* type,
    const std::function<Program* (const mesh::LodMesh&)>& programSelector,
    uint8_t kindBits,
    render::Batch& batch) noexcept
{
    if (m_instancer) {
        m_instancer->bindBatch(ctx, type, programSelector, kindBits, batch, *this);
    }
    else {
        const auto& snapshot = ctx.m_registry->m_activeSnapshotRegistry->getSnapshot(m_snapshotIndex);

        batch.addSnapshot(
            ctx,
            type,
            programSelector,
            kindBits,
            snapshot,
            m_entityIndex);
    }
}

const Snapshot& Node::getActiveSnapshot(Registry* registry) const noexcept
{
    return registry->m_activeSnapshotRegistry->getSnapshot(m_snapshotIndex);
}

Snapshot& Node::modifyActiveSnapshot(Registry* registry) noexcept
{
    return registry->m_activeSnapshotRegistry->modifySnapshot(m_snapshotIndex);
}

void Node::updateModelMatrix() noexcept
{
    if (m_parent) {
        m_state.updateModelMatrix(getParent()->getState());
    }
    else {
        m_state.updateRootMatrix();
    }
}

void Node::setTagMaterialIndex(int index)
{
    if (m_tagMaterialIndex != index) {
        m_tagMaterialIndex = index;
        m_state.m_dirtySnapshot = true;
        if (m_generator) {
            for (auto& state : m_generator->modifyStates()) {
                state.m_dirtySnapshot = true;
            }
        }
    }
}

void Node::setSelectionMaterialIndex(int index)
{
    if (m_selectionMaterialIndex != index) {
        m_selectionMaterialIndex = index;
        m_state.m_dirtySnapshot = true;
        if (m_generator) {
            for (auto& state : m_generator->modifyStates()) {
                state.m_dirtySnapshot = true;
            }
        }
    }
}

int Node::getHighlightIndex() const noexcept
{
    const auto& assets = Assets::get();

    if (assets.showHighlight) {
        if (assets.showTagged && m_tagMaterialIndex > -1) return m_tagMaterialIndex;
        if (assets.showSelection && m_selectionMaterialIndex > -1) return m_selectionMaterialIndex;
    }
    return -1;
}

ki::node_id Node::lua_getId() const noexcept
{
    return m_handle.m_id;
}

const std::string& Node::lua_getName() const noexcept
{
    return m_typeHandle.toType()->getName();
}

int Node::lua_getCloneIndex() const noexcept
{
    return m_cloneIndex;
}

const std::array<float, 3> Node::lua_getPos() const noexcept
{
    const auto& pos = m_state.getPosition();
    return { pos.x, pos.y, pos.z };
}
