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

#include "animation/Rig.h"
#include "animation/AnimationSystem.h"

#include "mesh/mesh_util.h"

#include "mesh/LodMesh.h"
#include "mesh/LodMeshInstance.h"
#include "mesh/ModelMesh.h"
#include "mesh/RegisteredRig.h"

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
#include "render/InstanceRegistry.h"

#include "render/Batch.h"

namespace {
    const static glm::mat4 IDENTITY_MATRIX{ 1.f };
}

namespace model
{
    Node::Node()
    {
    }

    Node::Node(Node&& o) noexcept
        : m_handle{ o.m_handle }
    {
    }

    Node& Node::operator=(Node&& o) noexcept
    {
        if (&o == this) return *this;
        return *this;
    }


    Node::~Node()
    {
        KI_INFO(fmt::format("NODE: delete - {}", str()));
        m_handle.m_id = 0;
        m_handle.m_handleIndex = 0;
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
        auto* type = m_typeHandle.toType();

        // TODO KI need to set these to *model::NodeType* in loader
        // => apply from there in NodeRegistry
        state.setPivotAlignment(type->m_pivotPoint.resolveAlignment(type));
        state.setPivotOffset(type->m_pivotPoint.m_offset);
        state.setFront(type->m_front);
        state.setBaseRotation(type->m_baseRotation);

        if (type->hasMesh()) {
            KI_DEBUG(fmt::format("ADD_ENTITY: {}", str()));

            state.setLocalVolume(type->getAABB().toLocalVolume());
            {
                state.m_flags = type->resolveEntityFlags();
            }
        }

        {
            // NOTE KI for now, allow only single Rig per mesh type
            // i.e. not possible to attach animated attachments
            // or have separate animations for LOD level meshes
            auto& animationsystem = animation::AnimationSystem::get();
            std::unordered_map<const animation::Rig*, int> rigPalettes;

            const auto& lodMeshes = type->getLodMeshes();

            for (int index = 0;  auto& lodMesh : lodMeshes) {
                const auto* mesh = lodMesh.getMesh<mesh::Mesh>();
                const auto* rig = mesh->getRig();
                if (!rig) continue;

                if (rigPalettes.contains(rig)) continue;
                rigPalettes.insert({ rig, index++ });

                auto& registeredRig = m_registeredRigs.emplace_back();
                registeredRig.m_rig = rig;
                registeredRig.m_rigRef = animationsystem.registerRig(*rig);
                registeredRig.m_socketRef = animationsystem.registerSockets(registeredRig.m_rigRef, *rig);
                registeredRig.m_jointRef = animationsystem.registerJoints(registeredRig.m_rigRef, rig->getJointContainer());
            }

            m_lodMeshInstances.reserve(lodMeshes.size());
            for (int index = 0;  auto& lodMesh : lodMeshes) {
                auto& lod = m_lodMeshInstances.emplace_back();
                lod.m_lodMeshIndex = index++;

                const auto* mesh = lodMesh.getMesh<mesh::Mesh>();
                auto* rig = mesh->getRig();
                if (!rig) continue;

                const auto& registeredRig = m_registeredRigs[rigPalettes[rig]];
                lod.m_socketBaseIndex = registeredRig.m_socketRef.offset;
                lod.m_jointBaseIndex = registeredRig.m_jointRef.offset;
            }
        }

        if (m_generator) {
            m_generator->prepareWT(ctx, *this);
        }

        if (m_particleGenerator) {
            m_particleGenerator->prepareWT();
        }

        auto& scriptSystem = script::ScriptSystem::get();

        if (m_typeFlags.root) {
            for (auto scriptId : type->getScripts())
            {
                scriptSystem.runGlobalScript(this, scriptId);
            }
        }
        else {
            if (!type->getScripts().empty()) {
                scriptSystem.bindNode(this);
            }
        }
    }

    void Node::unprepareWT(
        const PrepareContext& ctx,
        NodeState& state)
    {
        auto* type = m_typeHandle.toType();

        auto& animationSystem = animation::AnimationSystem::get();

        for (auto& registeredRig : m_registeredRigs) {
            animationSystem.unregisterJoints(registeredRig.m_jointRef);
            animationSystem.unregisterSockets(registeredRig.m_socketRef);
            animationSystem.unregisterRig(registeredRig.m_rigRef);
        }

        //if (m_generator) {
        //    m_generator->unprepareWT(ctx, *this);
        //}

        //if (m_particleGenerator) {
        //    m_particleGenerator->unprepareWT();
        //}

        auto& scriptSystem = script::ScriptSystem::get();

        if (m_typeFlags.root) {
            // TODO KI not possible "unrun" global scripts
        }
        else {
            if (!type->getScripts().empty()) {
                scriptSystem.unbindNode(this);
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

        registerDrawables();
    }

    void Node::updateVAO(const render::RenderContext& ctx) noexcept
    {
        if (m_generator) {
            m_generator->updateVAO(ctx, *this);
        }
    }

    void Node::registerDrawables() noexcept
    {
        auto& instanceRegistry = render::InstanceRegistry::get();

        if (m_generator && m_generator->isLightWeight()) {
            m_generator->registerDrawables(
                instanceRegistry,
                *this);
        }
        else {
            const auto* snapshot = getSnapshotRT();

            const auto* type = getType();
            const auto& lodMeshes = type->getLodMeshes();
            const auto& lodMeshInstances = m_lodMeshInstances;

            for (int i = 0; i < lodMeshInstances.size(); i++) {
                const auto& lod = lodMeshInstances[i];
                const auto& lodMesh = lodMeshes[i];
                if (!lodMesh.m_mesh)
                    continue;

                render::DrawableInfo drawable;
                {
                    drawable.lodMeshIndex = i;

                    drawable.meshId = lodMesh.getMesh<mesh::Mesh>()->m_id;

                    drawable.entityIndex = getEntityIndex();
                    drawable.materialIndex = lodMesh.m_materialIndex;
                    drawable.jointBaseIndex = lod.m_jointBaseIndex;

                    drawable.baseVertex = lodMesh.m_baseVertex;
                    drawable.baseIndex = lodMesh.m_baseIndex;
                    drawable.indexCount = lodMesh.m_indexCount;

                    drawable.minDistance2 = lodMesh.m_minDistance2;
                    drawable.maxDistance2 = lodMesh.m_maxDistance2;

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
                    drawable.worldVolume = snapshot->getWorldVolume();
                    drawable.localTransform = lodMesh.m_baseTransform;
                }

                auto index = instanceRegistry.registerDrawable(drawable);
                if (m_instanceRef.offset == 0) {
                    m_instanceRef.offset = index;
                }
                m_instanceRef.size++;
            }
        }
    }

    void Node::bindBatch(
        const render::RenderContext& ctx,
        const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
        const std::function<void(ki::program_id)>& programPrepare,
        uint8_t kindBits,
        render::Batch& batch) noexcept
    {
        if (m_generator && m_generator->isLightWeight()) {
            m_generator->bindBatch(
                ctx,
                programSelector,
                programPrepare,
                kindBits,
                batch,
                *this);
        }
        else {
            batch.addDrawablesSingleNode(
                ctx,
                getType(),
                m_instanceRef,
                programSelector,
                programPrepare,
                kindBits);
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
}
