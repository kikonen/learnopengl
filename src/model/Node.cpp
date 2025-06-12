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

#include "model/NodeType.h"
#include "model/EntityFlags.h"

#include "audio/Source.h"
#include "audio/Listener.h"

#include "script/ScriptSystem.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
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

    m_typeFlags = type->m_flags;
    m_layer = type->m_layer;

    // TODO KI need to set these to *NodeType* in loader
    // => apply from there in NodeRegistry
    state.setPivot(type->m_pivotPoint.resolve(type));
    state.setFront(type->m_front);
    state.setBaseRotation(type->m_baseRotation);

    if (type->hasMesh()) {
        KI_DEBUG(fmt::format("ADD_ENTITY: {}", str()));

        state.setVolume(type->getAABB().getVolume());
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

    if (!m_typeFlags.root) {
        auto& scriptSystem = script::ScriptSystem::get();

        for (auto scriptId : type->getScripts())
        {
            scriptSystem.bindNodeScript(this, scriptId);
        }
    }
}

void Node::prepareRT(
    const PrepareContext& ctx)
{
    const auto* snapshot = getSnapshotRT();
    if (!snapshot) return;

    if (m_camera) {
        m_camera->snapToIdeal(*snapshot);
    }
}

void Node::updateVAO(const RenderContext& ctx) noexcept
{
    if (m_generator) {
        m_generator->updateVAO(ctx, *this);
    }
}

void Node::bindBatch(
    const RenderContext& ctx,
    const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
    uint8_t kindBits,
    render::Batch& batch) noexcept
{
    const auto* snapshot = getSnapshotRT();
    if (!snapshot) return;

    if (m_generator && m_generator->isLightWeight()) {
        m_generator->bindBatch(
            ctx,
            programSelector,
            kindBits,
            batch,
            *this,
            *snapshot);
    }
    else {
        batch.addSnapshot(
            ctx,
            this,
            programSelector,
            kindBits,
            *snapshot,
            m_entityIndex);
    }
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
