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

#include "audio/Source.h"
#include "audio/Listener.h"

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
        "<NODE: id={}, name={}, type={}>",
        m_handle.str(), m_name, type ? type->str() : "<null>");
}

void Node::prepareWT(
    const PrepareContext& ctx,
    NodeState& state)
{
    auto& registry = ctx.m_registry;

    auto* type = m_typeHandle.toType();

    if (type->hasMesh()) {
        KI_DEBUG(fmt::format("ADD_ENTITY: {}", str()));

        {
            state.m_flags = type->resolveEntityFlags();
        }
    }

    {
        // NOTE KI for now, allow only single Rig per mesh type
        // i.e. not possible to attach animated attachments
        // or have separate animations for LOD level meshes
        const auto rig = mesh::findRig(type->modifyLodMeshes());

        if (rig) {
            auto [boneBaseIndex, socketBaseIndex] = animation::AnimationSystem::get().registerInstance(*rig);
            state.m_boneBaseIndex = boneBaseIndex;
            state.m_socketBaseIndex = socketBaseIndex;
            //m_state.m_animationClipIndex = ;
        }
    }

    if (m_generator) {
        m_generator->prepareWT(ctx, *this);
    }

    if (m_particleGenerator) {
        m_particleGenerator->prepareWT();
    }
}

void Node::prepareRT(
    const PrepareContext& ctx)
{
    auto* type = m_typeHandle.toType();

    const auto* snapshot = getSnapshotRT();
    if (!snapshot) return;

    if (m_camera) {
        m_camera->snapToIdeal(*snapshot);
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
    if (m_generator) {
        m_generator->updateVAO(ctx, *this);
    }
}

void Node::bindBatch(
    const RenderContext& ctx,
    mesh::MeshType* type,
    const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
    uint8_t kindBits,
    render::Batch& batch) noexcept
{
    const auto* snapshot = getSnapshotRT();
    if (!snapshot) return;

    if (m_generator && m_generator->isLightWeight()) {
        m_generator->bindBatch(
            ctx,
            type,
            programSelector,
            kindBits,
            batch,
            *this,
            *snapshot);
    }
    else {
        batch.addSnapshot(
            ctx,
            type,
            programSelector,
            kindBits,
            *snapshot,
            m_entityIndex);
    }
}

void Node::setTagMaterialIndex(int index)
{
    if (m_tagMaterialIndex != index) {
        m_tagMaterialIndex = index;
        getState().m_dirtySnapshot = true;
    }
}

void Node::setSelectionMaterialIndex(int index)
{
    if (m_selectionMaterialIndex != index) {
        m_selectionMaterialIndex = index;
        getState().m_dirtySnapshot = true;
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

audio::Source* Node::getAudioSource(audio::source_id id) const
{
    audio::Source* source{ nullptr };

    if (m_audioSources) {
        for (auto& src : *m_audioSources) {
            if (src.m_id == id) {
                return &src;
            }
        }
    }

    return nullptr;
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
    //return m_cloneIndex;
    return 0;
}

const std::array<float, 3> Node::lua_getPos() const noexcept
{
    const auto& pos = getState().getPosition();
    return { pos.x, pos.y, pos.z };
}
