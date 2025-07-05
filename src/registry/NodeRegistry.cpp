#include "NodeRegistry.h"

#include <algorithm>

#include <glm/gtx/quaternion.hpp>

#include <fmt/format.h>

#include "util/debug.h"
#include "util/thread.h"
#include "ki/limits.h"
#include "kigl/kigl.h"

#include "asset/Assets.h"

#include "shader/Program.h"

#include "model/Node.h"
#include "model/NodeType.h"
#include "model/CreateState.h"

#include "mesh/LodMesh.h"

#include "component/LightDefinition.h"
#include "component/Light.h"

#include "component/CameraDefinition.h"
#include "component/CameraComponent.h"

#include "component/FpsCamera.h"
#include "component/FollowCamera.h"
#include "component/OrbitCamera.h"
#include "component/SplineCamera.h"

#include "component/GeneratorDefinition.h"

#include "particle/ParticleDefinition.h"
#include "particle/ParticleGenerator.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "generator/NodeGenerator.h"

#include "component/TextDefinition.h"
#include "generator/TextGenerator.h"

#include "component/AudioSourceDefinition.h"
#include "component/AudioListenerDefinition.h"

#include "component/PhysicsDefinition.h"
#include "component/ControllerDefinition.h"

#include "audio/Listener.h"
#include "audio/Source.h"
#include "audio/AudioSystem.h"

#include "physics/PhysicsSystem.h"
#include "physics/physics_util.h"

#include "controller/PawnController.h"
#include "controller/CameraZoomController.h"

#include "render/DebugContext.h"

#include "script/ScriptSystem.h"

#include "EntitySSBO.h"

#include "Registry.h"
#include "NodeTypeRegistry.h"
#include "EntityRegistry.h"
#include "ModelRegistry.h"
#include "ControllerRegistry.h"

namespace {
    const std::vector<pool::NodeHandle> EMPTY_NODE_LIST;

    constexpr int NULL_INDEX = 0;
    constexpr int ID_INDEX = 1;

    constexpr int INITIAL_SIZE = 10000;

    const ki::program_id NULL_PROGRAM_ID = 0;

    const pool::NodeHandle NULL_HANDLE = pool::NodeHandle::NULL_HANDLE;

    static NodeRegistry* s_registry{ nullptr };


    std::unique_ptr<CameraComponent> createCameraComponent(
        const NodeType* type)
    {
        if (!type->m_cameraDefinition) return nullptr;

        const auto& data = *type->m_cameraDefinition;

        // NOTE only node cameras in scenefile for now
        std::unique_ptr<CameraComponent> component;

        switch (data.m_type) {
        case CameraType::fps: {
            auto c = std::make_unique<FpsCamera>();
            c->setPitch(glm::radians(data.m_pitch));
            c->setPitchSpeed(glm::radians(data.m_pitchSpeed));
            component = std::move(c);
            break;
        }
        case CameraType::follow: {
            auto c = std::make_unique<FollowCamera>();
            c->m_springConstant = data.m_springConstant;
            c->m_distance = data.m_distance;
            component = std::move(c);
            break;
        }
        case CameraType::orbit: {
            auto c = std::make_unique<OrbitCamera>();
            c->m_offset = data.m_offset;
            c->m_up = data.m_up;
            c->m_pitchSpeed = glm::radians(data.m_pitchSpeed);
            c->m_yawSpeed = glm::radians(data.m_yawSpeed);
            component = std::move(c);
            break;
        }
        case CameraType::spline: {
            auto c = std::make_unique<SplineCamera>();
            c->m_path = Spline{ data.m_path };
            c->m_speed = data.m_speed;
            component = std::move(c);
            break;
        }
        }

        component->m_enabled = true;
        component->m_default = data.m_default;

        {
            auto& camera = component->getCamera();
            if (data.m_orthogonal) {
                camera.setViewport(data.m_viewport);
            }
            camera.setAxis(data.m_front, data.m_up);
            camera.setFov(data.m_fov);
        }

        return component;
    }

    std::unique_ptr<Light> createLight(
        const NodeType* type)
    {
        if (!type->m_lightDefinition) return nullptr;

        const auto& data = *type->m_lightDefinition;

        auto light = std::make_unique<Light>();

        light->m_enabled = true;

        light->setTargetId(data.m_targetId);

        light->m_linear = data.m_linear;
        light->m_quadratic = data.m_quadratic;

        light->m_cutoffAngle = data.m_cutoffAngle;
        light->m_outerCutoffAngle = data.m_outerCutoffAngle;

        light->m_diffuse = data.m_diffuse;
        light->m_intensity = data.m_intensity;

        light->m_type = data.m_type;

        return light;
    }

    std::unique_ptr<particle::ParticleGenerator> createParticleGenerator(
        const NodeType* type)
    {
        if (!type->m_particleDefinition) return nullptr;
        auto generator = std::make_unique<particle::ParticleGenerator>();
        generator->setDefinition(*type->m_particleDefinition);
        return generator;
    }

    std::unique_ptr<TextGenerator> createTextGenerator(
        const NodeType* type)
    {
        if (!type->m_textDefinition) return nullptr;

        const auto& data = *type->m_textDefinition;

        const auto& assets = Assets::get();

        auto generator = std::make_unique<TextGenerator>();

        generator->setFontId(data.m_fontId);
        generator->setText(data.m_text);
        generator->setPivot(data.m_pivot);
        generator->setAlignHorizontal(data.m_alignHorizontal);
        generator->setAlignVertical(data.m_alignVertical);

        generator->m_material = *data.m_material;

        return generator;
    }

    std::unique_ptr<audio::Listener> createAudioListener(
        const NodeType* type)
    {
        if (!type->m_audioListenerDefinition) return nullptr;

        const auto& data = *type->m_audioListenerDefinition;

        std::unique_ptr<audio::Listener> listener = std::make_unique<audio::Listener>();

        listener->m_gain = data.m_gain;

        return listener;
    }

    void createAudioSource(
        const AudioSourceDefinition& data,
        audio::Source& source)
    {
        const auto& assets = Assets::get();

        source.m_id = data.m_sourceId;
        source.m_soundId = data.m_soundId;

        source.m_referenceDistance = data.m_referenceDistance;
        source.m_maxDistance = data.m_maxDistance;
        source.m_rolloffFactor = data.m_rolloffFactor;

        source.m_minGain = data.m_minGain;
        source.m_maxGain = data.m_maxGain;

        source.m_looping = data.m_looping;

        source.m_pitch = data.m_pitch;
        source.m_gain = data.m_gain;
    }

    std::unique_ptr<std::vector<audio::Source>> createAudioSources(
        const NodeType* type)
    {
        if (!type->m_audioSourceDefinitions) return nullptr;

        const auto& sources = *type->m_audioSourceDefinitions;

        auto result = std::make_unique<std::vector<audio::Source>>();

        for (const auto& data : sources) {
            auto& src = result->emplace_back();
            createAudioSource(data, src);
            if (!src.m_soundId) {
                result->pop_back();
            }
        }
        return result->empty() ? nullptr : std::move(result);
    }

    std::unique_ptr<NodeController> createController(
        ControllerDefinition& definition)
    {
        switch (definition.m_type) {
        case ControllerType::pawn: {
            return std::make_unique<PawnController>();
        }
        case ControllerType::camera_zoom: {
            return std::make_unique<CameraZoomController>();
        }
        }

        return nullptr;
    }
}

void NodeRegistry::init() noexcept
{
    assert(!s_registry);
    s_registry = new NodeRegistry();
}

void NodeRegistry::release() noexcept
{
    auto* s = s_registry;
    s_registry = nullptr;
    delete s;
}

NodeRegistry& NodeRegistry::get() noexcept
{
    assert(s_registry);
    return *s_registry;
}

NodeRegistry::NodeRegistry()
{
    clear();
}

NodeRegistry::~NodeRegistry()
{
    //std::lock_guard lock(m_lock);

    {
        m_activeNode.reset();

        m_rootRT.reset();;
        m_rootWT.reset();;
    }

    //m_nodes.clear();
    //m_parentIndeces.clear();
    //m_cachedNodesWT.clear();
    //m_cachedNodesRT.clear();
    //m_entities.clear();

    m_skybox.reset();

    pool::NodeHandle::clear();
}

void NodeRegistry::clear()
{
    m_skybox.reset();

    m_rootWT.reset();
    m_rootRT.reset();

    m_rootIndex = 0;

    m_handles.clear();
    m_parentIndeces.clear();
    m_states.clear();
    m_snapshotsPending.clear();
    m_snapshotsRT.clear();
    m_entities.clear();
    m_dirtyEntities.clear();

    m_cachedNodesWT.clear();
    m_cachedNodesRT.clear();

    m_nodeLevel = 0;
    m_cachedNodeLevelWT = 0;
    m_cachedNodeLevelRT = 0;

    m_activeNode.reset();

    {
        m_handles.reserve(INITIAL_SIZE);
        m_parentIndeces.reserve(INITIAL_SIZE);
        m_states.reserve(INITIAL_SIZE);
        m_snapshotsPending.reserve(INITIAL_SIZE);
        m_snapshotsRT.reserve(INITIAL_SIZE);
        m_entities.reserve(INITIAL_SIZE);
        m_dirtyEntities.reserve(INITIAL_SIZE);

        m_cachedNodesWT.reserve(INITIAL_SIZE);
        m_cachedNodesRT.reserve(INITIAL_SIZE);

        m_freeIndeces.reserve(INITIAL_SIZE);
        m_pendingAdded.reserve(INITIAL_SIZE);
        m_pendingRemoved.reserve(INITIAL_SIZE);
    }

    // NOTE KI keep NULL and IDENTITY as separate since
    // logic *can* (and thus *will*) accidentally modify null state
    // => that does not matter as long as NULL is not tried to be used
    //    as identity matrix
    // => in GPU side NULL state *may* (thus *will*) appear as entity
    // => should somehow make NULL state immutable
    {
        // NOTE KI declare index == 0 as NULL object
        auto& state = m_states.emplace_back();
        state.updateRootMatrix();

        m_handles.emplace_back();
        m_parentIndeces.push_back(0);
    }

    {
        // NOTE KI declare index == 1 as IDENTITY object
        auto& state = m_states.emplace_back();
        state.updateRootMatrix();

        m_handles.emplace_back();
        m_parentIndeces.push_back(0);
    }
}

void NodeRegistry::prepare(
    Registry* registry)
{
    const auto& assets = Assets::get();

    m_registry = registry;

    m_rootId = assets.rootId;

    attachListeners();
}

void NodeRegistry::updateWT(const UpdateContext& ctx)
{
    auto& cachedNodes = getCachedNodesWT();

    {
        auto& physicsSystem = physics::PhysicsSystem::get();

        {
            auto& state = m_states[NULL_INDEX];
            state.updateRootMatrix();
        }
        {
            auto& state = m_states[ID_INDEX];
            state.updateRootMatrix();
        }
        {
            auto& state = m_states[m_rootIndex];
            state.updateRootMatrix();
        }

        //std::lock_guard lock(m_lock);
        // NOTE KI nodes are in DAG order
        for (int i = m_rootIndex + 1; i < m_states.size(); i++) {
            auto* node = cachedNodes[i];
            if (!node) continue;

            auto& state = m_states[i];
            const auto& parentState = m_states[m_parentIndeces[i]];
            auto* generator = node->m_generator.get();

            state.updateModelMatrix(parentState);

            if (generator) {
                generator->updateWT(ctx, *node);
                state.updateModelMatrix(parentState);
            }

            if (physicsSystem.isEnabled() &&
                (!generator || !generator->isLightWeight()))
            {
                if (node->m_typeFlags.dynamicBounds || node->m_typeFlags.staticBounds)
                {
                    updateBounds(ctx, state, parentState, node, physicsSystem);
                    state.updateModelMatrix(parentState);
                }
            }
        }
    }
}

void NodeRegistry::postUpdateWT(const UpdateContext& ctx)
{
    auto& cachedNodes = getCachedNodesWT();

    for (int i = m_rootIndex + 1; i < m_states.size(); i++) {
        auto* node = cachedNodes[i];
        if (!node) continue;

        auto& state = m_states[i];

        if (node->m_particleGenerator) {
            node->m_particleGenerator->updateWT(ctx, *node);
        }

        if (node->m_audioListener) {
            if (node->m_handle == m_activeNode) {
                node->m_audioListener->updateActive(state);
            }
        }

        if (node->m_audioSources) {
            for (auto& src : *node->m_audioSources) {
                src.update(state);
            }
        }
    }
}

int NodeRegistry::validateModelMatrices()
{
    for (auto i = m_rootIndex + 1; i < m_states.size(); i++) {
        const auto parentLevel = m_states[m_parentIndeces[i]].m_matrixLevel;
        if (!m_states[i].valid(parentLevel)) return i;
    }
    return -1;
}

void NodeRegistry::updateModelMatrices()
{
    {
        auto& state = m_states[NULL_INDEX];
        state.updateRootMatrix();
    }
    {
        auto& state = m_states[ID_INDEX];
        state.updateRootMatrix();
    }
    {
        auto& state = m_states[m_rootIndex];
        state.updateRootMatrix();
    }

    for (auto i = m_rootIndex + 1; i < m_states.size(); i++) {
        assert(m_parentIndeces[i] >= m_rootIndex);
        m_states[i].updateModelMatrix(m_states[m_parentIndeces[i]]);
    }
}

void NodeRegistry::updateModelMatrices(const Node* node)
{
    auto index = node->m_entityIndex;
    m_states[index].updateModelMatrix(m_states[m_parentIndeces[index]]);
}

void NodeRegistry::snapshotPending()
{
    std::lock_guard lock(m_snapshotLock);

    {
        const auto sz = m_states.size();
        const auto forceAfter = m_snapshotsPending.size() - 1;

        m_snapshotsPending.resize(sz);

        for (int i = 0; i < sz; i++) {
            const auto& state = m_states[i];
            assert(!state.m_dirty);

            if (i >= forceAfter || state.m_dirtySnapshot) {
                m_snapshotsPending[i].applyFrom(state);
            }
        }
    }

    {
        auto& dbg = render::DebugContext::modify();
        dbg.m_physicsMeshesPending.exchange(dbg.m_physicsMeshesWT);

        std::shared_ptr<std::vector<mesh::MeshInstance>> tmp;
        dbg.m_physicsMeshesWT.store(tmp);
    }
}

void NodeRegistry::snapshotRT()
{
    std::lock_guard lock(m_snapshotLock);

    snapshot(m_snapshotsPending, m_snapshotsRT);
    cacheNodes(m_cachedNodesRT, m_cachedNodeLevelRT);

    m_entities.resize(m_snapshotsRT.size());
    m_dirtyEntities.resize(m_snapshotsRT.size());

    auto& dbg = render::DebugContext::modify();

    if (dbg.m_physicsMeshesPending.load()) {
        dbg.m_physicsMeshesRT.exchange(dbg.m_physicsMeshesPending);

        std::shared_ptr<std::vector<mesh::MeshInstance>> tmp;
        dbg.m_physicsMeshesPending.store(tmp);
    }
}

void NodeRegistry::snapshot(
    std::vector<Snapshot>& src,
    std::vector<Snapshot>& dst)
{
    const auto sz = src.size();
    const auto forceAfter = dst.size() - 1;

    dst.resize(sz);

    for (int i = 0; i < sz; i++) {
        const auto& snapshot = src[i];
        if (i >= forceAfter || snapshot.m_dirty) {
            dst[i].applyFrom(snapshot);
        }
    }
}

std::vector<Node*>& NodeRegistry::getCachedNodesWT()
{
    cacheNodes(m_cachedNodesWT, m_cachedNodeLevelWT);
    return m_cachedNodesWT;
}

void NodeRegistry::prepareUpdateRT(const UpdateContext& ctx)
{
    snapshotRT();
}

void NodeRegistry::updateRT(const UpdateContext& ctx)
{
    m_rootRT = m_rootWT;

    auto* root = getRootRT();
    m_rootPreparedRT = root && root->m_preparedRT;
}

std::pair<int, int> NodeRegistry::updateEntity(const UpdateContext& ctx)
{
    //std::lock_guard lock(m_lock);
    int minDirty = INT32_MAX;
    int maxDirty = INT32_MIN;

    for (int i = 0; i < m_snapshotsRT.size(); i++) {
        auto* node = m_cachedNodesRT[i];
        const auto& state = m_states[i];
        const auto& snapshot = m_snapshotsRT[i];

        if (!snapshot.m_dirty) continue;

        auto& entity = m_entities[i];

        if (node) {
            entity.u_objectID = node->getId();

            auto* textGenerator = node->getGenerator<TextGenerator>();
            if (textGenerator) {
                entity.u_fontHandle = textGenerator->getAtlasTextureHandle();
            }
        }

        entity.u_tilingX = state.m_tilingX;
        entity.u_tilingY = state.m_tilingY;

        snapshot.updateEntity(entity);
        snapshot.m_dirty = false;

        m_dirtyEntities[i] = true;
        if (i < minDirty) minDirty = i;
        if (i > maxDirty) maxDirty = i;
    }

    return { minDirty, maxDirty };
}

void NodeRegistry::cacheNodes(
    std::vector<Node*>& cache,
    ki::level_id& cacheLevel)
{
    if (cacheLevel == m_nodeLevel) return;
    cacheLevel = m_nodeLevel;

    cache.resize(m_handles.size());
    for (size_t i = 0; i < m_handles.size(); i++) {
        cache[i] = m_handles[i].toNode();
    }
}

void NodeRegistry::attachListeners()
{
    const auto& assets = Assets::get();

    auto* dispatcher = m_registry->m_dispatcherWorker;
    auto* dispatcherView = m_registry->m_dispatcherView;

    dispatcher->addListener(
        event::Type::node_add,
        [this](const event::Event& e) {
            attachNode(
                pool::NodeHandle::toHandle(e.body.node.target),
                pool::NodeHandle::toHandle(e.body.node.parentId),
                e.blob->body.state);
        });

    dispatcher->addListener(
        event::Type::node_remove,
        [this](const event::Event& e) {
            detachNode(
                pool::NodeHandle::toHandle(e.body.node.target));
        });

    dispatcher->addListener(
        event::Type::node_dispose,
        [this](const event::Event& e) {
            disposeNode(
                pool::NodeHandle::toHandle(e.body.node.target));
        });

    dispatcher->addListener(
        event::Type::node_activate,
        [this](const event::Event& e) {
            setActiveNode(pool::NodeHandle::toHandle(e.body.node.target));
        });

    if (assets.useScript) {
        dispatcher->addListener(
            event::Type::node_added,
            [this](const event::Event& e) {
                auto& data = e.body.node;
                auto nodeHandle = pool::NodeHandle::toHandle(data.target);

                const auto* node = nodeHandle.toNode();
                if (!node) return;

                const auto* type = node->getType();
                const auto nodeId = data.target;

                auto& scriptSystem = script::ScriptSystem::get();

                for (const auto& scriptId : type->getScripts()) {
                    if (nodeHandle == m_rootHandle) {
                        //scriptSystem.runGlobalScript(node, scriptId);
                    } else
                    {
                        event::Event evt { event::Type::script_run };
                        auto& body = evt.body.script = {
                            .target = nodeId,
                            .id = scriptId,
                        };
                        m_registry->m_dispatcherWorker->send(evt);
                    }
                }
            });
    }

    dispatcherView->addListener(
        event::Type::type_prepare_view,
        [this](const event::Event& e) {
            auto& data = e.body.nodeType;
            auto* type = pool::TypeHandle::toType(data.target);
            if (!type) return;
            type->prepareRT({ m_registry });
        });
}

void NodeRegistry::handleNodeAdded(Node* node)
{
    if (!node) return;

    auto nodeHandle = node->toHandle();

    node->prepareRT({ m_registry });

    if (node->m_generator) {
        const PrepareContext ctx{ m_registry };
        node->m_generator->prepareRT(ctx, *node);
    }
    node->m_preparedRT = true;
}

void NodeRegistry::handleNodeRemoved(Node* node)
{
    if (!node) return;

    auto nodeHandle = node->toHandle();

    if (node->m_generator) {
        //const PrepareContext ctx{ m_registry };
        //node->m_generator->unprepareRT(ctx, *node);
    }
}

void NodeRegistry::notifyPendingChanges()
{
    // NOTE KI ensure related snapshots are visible in RT
    // => otherwise IOOBE will trigger
    //m_registry->m_pendingSnapshotRegistry->copyFrom(
    //    m_registry->m_workerSnapshotRegistry,
    //    node->m_snapshotIndex, 1);

    for (auto& nodeHandle : m_pendingAdded) {
        auto* node = nodeHandle.toNode();
        if (!node) continue;

        {
            event::Event evt{ event::Type::node_added };
            evt.body.node.target = nodeHandle;
            m_registry->m_dispatcherWorker->send(evt);
        }

        // NOTE KI type must be prepared *before* node
        {
            event::Event evt{ event::Type::type_prepare_view };
            evt.body.nodeType.target = node->m_typeHandle.toId();
            m_registry->m_dispatcherView->send(evt);
        }

        {
            event::Event evt{ event::Type::node_added };
            evt.body.node.target = nodeHandle;
            m_registry->m_dispatcherView->send(evt);
        }

        if (node->m_typeFlags.skybox)
        {
            event::Event evt{ event::Type::type_prepare_view };
            evt.body.nodeType.target = node->m_typeHandle.toId();
            m_registry->m_dispatcherView->send(evt);
        }
    }

    m_pendingAdded.clear();

    for (auto& nodeHandle : m_pendingRemoved) {
        auto* node = nodeHandle.toNode();
        if (!node) continue;

        {
            event::Event evt{ event::Type::node_select };
            evt.body.select = {
                .target = nodeHandle.toId(),
                .select = false,
                .append = true
            };
            m_registry->m_dispatcherView->send(evt);
        }

        {
            event::Event evt{ event::Type::node_removed };
            evt.body.node.target = nodeHandle;
            m_registry->m_dispatcherWorker->send(evt);
        }

        {
            event::Event evt{ event::Type::node_removed };
            evt.body.node.target = nodeHandle;
            m_registry->m_dispatcherView->send(evt);
        }
    }

    m_pendingRemoved.clear();
}

void NodeRegistry::attachNode(
    const pool::NodeHandle nodeHandle,
    ki::node_id parentId,
    const CreateState& state) noexcept
{
    auto* node = nodeHandle.toNode();

    if (!node) {
        KI_WARN_OUT(fmt::format(
            "NODE_REGISTRY: IGNORE: missing node - node={}, parent={}",
            (int)nodeHandle, parentId));
        return;
    }

    if (nodeHandle.toId() == m_rootId) {
        m_rootHandle = nodeHandle;
    }

    // NOTE KI special case allow 0 to refer to "ROOT"
    if (!parentId) {
        parentId = m_rootId;
    }

    assert(node);
    assert(parentId || nodeHandle == m_rootHandle);

    // NOTE KI id != index
    //if (nodeId != m_rootId) {
    //    assert(parentId >= m_rootIndex);
    //}

    bindNode(nodeHandle, state);

    // NOTE KI ignore nodes with invalid parent
    if (!bindParent(nodeHandle, parentId)) {
        KI_WARN_OUT(fmt::format(
            "IGNORE: MISSING parent - parentId={}, node={}",
            parentId, node->str()));
        return;
    }

    auto* type = node->m_typeHandle.toType();

    type->prepareWT({ m_registry });
    node->prepareWT({ m_registry }, m_states[node->m_entityIndex]);

    if (type->m_physicsDefinition &&
        !(node->m_generator &&
        node->m_generator->isLightWeightPhysics()))
    {
        const auto& df = *type->m_physicsDefinition;
        physics::Object obj;
        {
            obj.m_body = df.m_body;
            obj.m_geom = df.m_geom;
        }

        auto& physicsSystem = physics::PhysicsSystem::get();
        node->m_physicsObjectId = physicsSystem.registerObject(
            node->m_handle,
            node->m_entityIndex,
            df.m_update,
            std::move(obj));
    }

    if (auto& sources = node->m_audioSources; sources) {
        for (auto& src : *sources) {
            audio::AudioSystem::get().prepareSource(src);
        }
    }

    if (type->m_controllerDefinitions) {
        for (auto& definition : *type->m_controllerDefinitions) {
            auto controller = createController(definition);
            if (!controller) continue;
            ControllerRegistry::get().addController(node->m_handle, std::move(controller));
        }
    }

    if (node->m_typeFlags.skybox) {
        bindSkybox(node->toHandle());
    }

    m_pendingAdded.push_back(nodeHandle);
}

void NodeRegistry::detachNode(
    const pool::NodeHandle nodeHandle) noexcept
{
    if (nodeHandle == m_rootHandle) return;

    auto* node = nodeHandle.toNode();

    if (!node) {
        KI_WARN_OUT(fmt::format(
            "NODE_REGISTRY: IGNORE: missing node - node={}",
            (int)nodeHandle));
        return;
    }

    if (!unbindParent(nodeHandle)) {
        KI_WARN_OUT(fmt::format(
            "ERROR: parent unbind failed - node={}",
            node->str()));
        return;
    }

    node->unprepareWT({ m_registry }, m_states[node->m_entityIndex]);

    const auto* type = node->m_typeHandle.toType();

    if (node->m_physicsObjectId)
    {
        auto& physicsSystem = physics::PhysicsSystem::get();
        physicsSystem.unregisterObject(nodeHandle);
    }

    unbindNode(nodeHandle);

    //node->unprepareWT({ m_registry }, m_states[node->m_entityIndex]);
}

void NodeRegistry::disposeNode(
    const pool::NodeHandle nodeHandle) noexcept
{
    uint32_t entityIndex = 0;
    {
        auto* node = nodeHandle.toNode();
        if (!node) return;

        entityIndex = entityIndex = node->m_entityIndex;
    }

    nodeHandle.release();

    {
        auto* node = nodeHandle.toNode();
        if (node) {
            KI_CRITICAL(fmt::format("NODE: failed to release: handle={}", nodeHandle.str()));
            return;
        }
    }

    // NOTE KI after dispose slot can be re-used
    m_freeIndeces.push_back(entityIndex);
}

void NodeRegistry::changeParent(
    const pool::NodeHandle nodeHandle,
    ki::node_id parentId) noexcept
{
    bindParent(nodeHandle, parentId);
}

void NodeRegistry::bindNode(
    const pool::NodeHandle nodeHandle,
    const CreateState& createState)
{
    Node* node = nodeHandle.toNode();
    if (!node) return;

    KI_INFO(fmt::format("BIND_NODE: {}", node->str()));

    node->m_entityIndex = static_cast<uint32_t>(m_handles.size());

    {
        const auto* type = node->getType();

        node->m_typeFlags = type->m_flags;
        node->m_layer = type->m_layer;

        node->m_camera = createCameraComponent(type);
        node->m_light = createLight(type);
        node->m_particleGenerator = createParticleGenerator(type);
        node->m_generator = GeneratorDefinition::createGenerator(type);
        node->m_audioSources = createAudioSources(type);
        node->m_audioListener = createAudioListener(type);
        node->m_camera = createCameraComponent(type);
        if (node->m_typeFlags.text) {
            node->m_generator = createTextGenerator(type);
        }
    }

    {
        NodeState state;
        {
            const auto* type = node->getType();

            state.setBaseScale(type->m_baseScale);
            state.setPosition(createState.m_position);
            state.setScale(createState.m_scale);
            state.setRotation(createState.m_rotation);
            state.m_tilingX = createState.m_tilingX;
            state.m_tilingY = createState.m_tilingY;
        }

        std::lock_guard lock(m_snapshotLock);

        m_handles.push_back(nodeHandle);
        m_parentIndeces.push_back(0);
        m_states.push_back(state);

        m_nodeLevel++;

        if (nodeHandle == m_rootHandle) {
            assert(!m_rootWT);
            m_rootWT = nodeHandle;
            m_rootIndex = node->m_entityIndex;
        }
    }

    KI_INFO(fmt::format("ATTACH_NODE: node={}", node->str()));
}

void NodeRegistry::unbindNode(
    const pool::NodeHandle nodeHandle)
{
    Node* node = nodeHandle.toNode();
    if (!node) return;

    KI_INFO(fmt::format("UNBIND_NODE: {}", node->str()));

    // TODO KI controllers, etc.?!?
    //{
    //    const auto* type = node->getType();

    //    node->m_camera = createCameraComponent(type);
    //    node->m_light = createLight(type);
    //    node->m_particleGenerator = createParticleGenerator(type);
    //    node->m_generator = GeneratorDefinition::createGenerator(type);
    //    node->m_audioSources = createAudioSources(type);
    //    node->m_audioListener = createAudioListener(type);
    //    node->m_camera = createCameraComponent(type);
    //    if (node->m_typeFlags.text) {
    //        node->m_generator = createTextGenerator(type);
    //    }
    //}

    {
        std::lock_guard lock(m_snapshotLock);

        const auto entityIndex = node->m_entityIndex;

        m_handles[entityIndex] = pool::NodeHandle::NULL_HANDLE;
        m_parentIndeces[entityIndex] = 0;
        m_states[entityIndex] = {};
        m_snapshotsPending[entityIndex] = {};
        m_snapshotsRT[entityIndex] = {};
        m_entities[entityIndex] = {};
        m_dirtyEntities[entityIndex] = false;
        m_cachedNodesWT[entityIndex] = nullptr;
        m_cachedNodesRT[entityIndex] = nullptr;

        m_nodeLevel++;
    }

    m_pendingRemoved.push_back(nodeHandle);
}

bool NodeRegistry::bindParent(
    const pool::NodeHandle nodeHandle,
    ki::node_id parentId)
{
    // NOTE KI everything else, except root requires parent
    if (nodeHandle == m_rootHandle) return true;

    auto parentHandle = pool::NodeHandle::toHandle(parentId);

    auto* node = nodeHandle.toNode();
    auto* parent = parentHandle.toNode();

    if (!node || !parent) return false;

    assert(parent->m_entityIndex >= m_rootIndex);
    assert(parent->m_entityIndex < node->m_entityIndex);

    m_parentIndeces[node->m_entityIndex] = parent->m_entityIndex;

    KI_INFO(fmt::format(
        "BIND_PARENT: parent={}, child={}",
        (int)parentHandle, (int)nodeHandle));

    return true;
}

bool NodeRegistry::unbindParent(
    const pool::NodeHandle nodeHandle)
{
    if (nodeHandle == m_rootHandle) return true;

    auto* node = nodeHandle.toNode();
    if (!node) return false;

    auto parentIndex = m_parentIndeces[node->m_entityIndex];
    auto parentHandle = m_handles[parentIndex];

    m_parentIndeces[node->m_entityIndex] = 0;

    KI_INFO(fmt::format(
        "UNBIND_PARENT: parent={}, child={}",
        (int)parentHandle, (int)nodeHandle));

    return true;
}

//void NodeRegistry::withLock(const std::function<void(NodeRegistry&)>& fn)
//{
//    std::lock_guard lock(m_lock);
//    fn(*this);
//}

void NodeRegistry::setActiveNode(pool::NodeHandle handle)
{
    if (!handle) return;
    auto* node = handle.toNode();
    if (!node) return;

    m_activeNode = handle;
}

void NodeRegistry::bindSkybox(
    pool::NodeHandle handle) noexcept
{
    auto* node = handle.toNode();
    if (!node) return;

    auto* type = node->m_typeHandle.toType();

    type->prepareWT({ m_registry });
    node->prepareWT({ m_registry }, m_states[node->m_entityIndex]);

    m_skybox = handle;
}

void NodeRegistry::updateBounds(
    const UpdateContext& ctx,
    NodeState& state,
    const NodeState& parentState,
    const Node* node,
    const physics::PhysicsSystem& physicsSystem)
{
    if (state.boundStaticDone) return;

    const glm::vec3 dir{ 0, -1, 0 };
    const uint32_t boundsMask = physics::mask(physics::Category::terrain);

    const auto& [success, level] = physicsSystem.getWorldSurfaceLevel(
        state.getWorldPosition(),
        dir,
        boundsMask);

    if (success) {
        if (node->m_typeFlags.staticBounds) {
            state.boundStaticDone = true;
        }
        const auto y = level - parentState.getWorldPosition().y;
        glm::vec3 newPos = state.getPosition();
        newPos.y = y + 0.75f - 0.35f;
        //KI_INFO(fmt::format("NODE_LEVEL: node={}, level={}", node.m_name, y));
        state.setPosition(newPos);
    }
}

void NodeRegistry::logDebugInfo(const std::string& err, uint32_t entityIndex) const
{
    if (entityIndex >= 0 && entityIndex < m_handles.size()) {
        auto* node = m_handles[entityIndex].toNode();
        KI_INFO_OUT(fmt::format("{}: index={}, node={}", err, entityIndex, node->str()));
    }
    else {
        KI_INFO_OUT(fmt::format("{}: index={}, node={}", err, entityIndex, "N/A"));
    }
}
