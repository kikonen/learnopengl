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

        //auto [targetId, targetResolvedSID] = resolveNodeId(data.targetBaseId, cloneIndex, tile);
        //light->setTargetId(targetId);

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
        m_activeCameraNode.reset();
        m_cameraNodes.clear();

        m_dirLightNodes.clear();
        m_pointLightNodes.clear();
        m_spotLightNodes.clear();

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
    m_snapshotsWT.clear();
    m_snapshotsPending.clear();
    m_snapshotsRT.clear();
    m_entities.clear();
    m_dirtyEntities.clear();

    m_cachedNodesWT.clear();
    m_cachedNodesRT.clear();

    m_nodeLevel = 0;
    m_cachedNodeLevel = 0;

    m_cameraNodes.clear();

    m_dirLightNodes.clear();
    m_pointLightNodes.clear();
    m_spotLightNodes.clear();

    m_activeNode.reset();
    m_activeCameraNode.reset();

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
    if (m_cachedNodeLevel != m_nodeLevel) {
        cacheNodes(m_cachedNodesWT);
        m_cachedNodeLevel = m_nodeLevel;
    }

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
            auto* node = m_cachedNodesWT[i];
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
    for (int i = m_rootIndex + 1; i < m_states.size(); i++) {
        auto* node = m_cachedNodesWT[i];
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

void NodeRegistry::snapshotWT()
{
    //std::lock_guard lock(m_snapshotLock);

    const auto sz = m_states.size();
    const auto forceAfter = m_snapshotsWT.size() - 1;

    m_snapshotsWT.resize(sz);

    for (int i = 0; i < sz; i++) {
        const auto& state = m_states[i];
        assert(!state.m_dirty);

        if (i >= forceAfter || state.m_dirtySnapshot) {
            m_snapshotsWT[i].applyFrom(state);
        }
    }
}

void NodeRegistry::snapshotPending()
{
    std::lock_guard lock(m_snapshotLock);

    snapshot(m_snapshotsWT, m_snapshotsPending);

    auto& dbg = render::DebugContext::modify();

    {
        dbg.m_physicsMeshesPending.exchange(dbg.m_physicsMeshesWT);

        std::shared_ptr<std::vector<mesh::MeshInstance>> tmp;
        dbg.m_physicsMeshesWT.store(tmp);
    }
}

void NodeRegistry::snapshotRT()
{
    std::lock_guard lock(m_snapshotLock);

    snapshot(m_snapshotsPending, m_snapshotsRT);

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

void NodeRegistry::prepareUpdateRT(const UpdateContext& ctx)
{
    snapshotRT();
    cacheNodesRT();
}

void NodeRegistry::updateRT(const UpdateContext& ctx)
{
    m_rootRT = m_rootWT;

    auto* root = getRootRT();
    m_rootPreparedRT = root && root->m_preparedRT;

    for (auto& handle : m_cameraNodes) {
        auto* node = handle.toNode();
        if (!node) continue;
        node->m_camera->updateRT(ctx, *node);
    }
    for (auto& handle : m_pointLightNodes) {
        auto* node = handle.toNode();
        if (!node) continue;
        node->m_light->updateRT(ctx, *node);
    }
    for (auto& handle : m_spotLightNodes) {
        auto* node = handle.toNode();
        if (!node) continue;
        node->m_light->updateRT(ctx, *node);
    }
    for (auto& handle : m_dirLightNodes) {
        auto* node = handle.toNode();
        if (!node) continue;
        node->m_light->updateRT(ctx, *node);
    }
}

std::pair<int, int> NodeRegistry::updateEntity(const UpdateContext& ctx)
{
    //std::lock_guard lock(m_lock);
    int minDirty = INT32_MAX;
    int maxDirty = INT32_MIN;

    for (int i = 0; i < m_snapshotsRT.size(); i++) {
        auto* node = m_cachedNodesRT[i];
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

        snapshot.updateEntity(entity);
        snapshot.m_dirty = false;

        m_dirtyEntities[i] = true;
        if (i < minDirty) minDirty = i;
        if (i > maxDirty) maxDirty = i;
    }

    return { minDirty, maxDirty };
}

void NodeRegistry::cacheNodesWT()
{
    cacheNodes(m_cachedNodesWT);
}

void NodeRegistry::cacheNodesRT()
{
    std::lock_guard lock(m_snapshotLock);
    cacheNodes(m_cachedNodesRT);
}

void NodeRegistry::cacheNodes(std::vector<Node*>& cache)
{
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
            auto handle = pool::NodeHandle::toHandle(e.body.node.target);
            auto* node = handle.toNode();
            if (!node) return;

            attachNode(
                e.body.node.target,
                e.body.node.parentId,
                e.blob->body.state);
        });

    dispatcher->addListener(
        event::Type::node_activate,
        [this](const event::Event& e) {
            setActiveNode(pool::NodeHandle::toHandle(e.body.node.target));
        });

    dispatcher->addListener(
        event::Type::camera_activate,
        [this](const event::Event& e) {
            auto& data = e.body.node;
            auto handle = pool::NodeHandle::toHandle(data.target);
            if (!handle) handle = findDefaultCameraNode();
            setActiveCameraNode(handle);
        });

    if (assets.useScript) {
        dispatcher->addListener(
            event::Type::node_added,
            [this](const event::Event& e) {
                auto& data = e.body.node;
                auto handle = pool::NodeHandle::toHandle(data.target);

                const auto* node = handle.toNode();
                if (!node) return;

                const auto* type = node->getType();
                const auto nodeId = data.target;

                auto& scriptSystem = script::ScriptSystem::get();

                for (const auto& scriptId : type->getScripts()) {
                    if (nodeId == m_rootId) {
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

    auto handle = node->toHandle();

    node->prepareRT({ m_registry });

    if (node->m_generator) {
        const PrepareContext ctx{ m_registry };
        node->m_generator->prepareRT(ctx, *node);
    }
    node->m_preparedRT = true;

    if (node->m_camera) {
        m_cameraNodes.push_back(handle);
        if (!m_activeCameraNode && node->m_camera->isDefault()) {
            setActiveCameraNode(handle);
        }
    }

    if (node->m_light) {
        Light* light = node->m_light.get();

        if (light->isDirectional()) {
            m_dirLightNodes.push_back(handle);
        }
        else if (light->isPoint()) {
            m_pointLightNodes.push_back(handle);
        }
        else if (light->isSpot()) {
            m_spotLightNodes.push_back(handle);
        }
    }
}

void NodeRegistry::attachNode(
    const ki::node_id nodeId,
    ki::node_id parentId,
    const CreateState& state) noexcept
{
    auto* node = pool::NodeHandle::toNode(nodeId);

    // NOTE KI special case allow 0 to refer to "ROOT"
    if (!parentId) {
        parentId = m_rootId;
    }

    assert(node);
    assert(parentId || nodeId == m_rootId);

    // NOTE KI id != index
    //if (nodeId != m_rootId) {
    //    assert(parentId >= m_rootIndex);
    //}

    bindNode(nodeId, state);

    // NOTE KI ignore nodes with invalid parent
    if (!bindParent(nodeId, parentId)) {
        KI_WARN_OUT(fmt::format("IGNORE: MISSING parent - parentId={}, node={}", parentId, node->str()));
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
        auto& physicsEngine = physics::PhysicsSystem::get();

        physics::Object obj;
        obj.m_body = df.m_body;
        obj.m_geom = df.m_geom;
        physicsEngine.registerObject(node->m_handle, node->m_entityIndex, df.m_update, std::move(obj));
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
        return bindSkybox(node->toHandle());
    }
}

void NodeRegistry::changeParent(
    const ki::node_id nodeId,
    const ki::node_id parentId) noexcept
{
    bindParent(nodeId, parentId);
}

void NodeRegistry::bindNode(
    const ki::node_id nodeId,
    const CreateState& createState)
{
    Node* node = pool::NodeHandle::toNode(nodeId);
    if (!node) return;

    pool::NodeHandle handle = node->toHandle();

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
        }

        std::lock_guard lock(m_snapshotLock);

        m_handles.push_back(handle);
        m_parentIndeces.push_back(0);
        m_states.push_back(state);
    }

    {
        m_nodeLevel++;

        if (nodeId == m_rootId) {
            assert(!m_rootWT);
            m_rootWT = handle;
            m_rootIndex = node->m_entityIndex;
        }
    }

    // NOTE KI ensure related snapshots are visible in RT
    // => otherwise IOOBE will trigger
    //m_registry->m_pendingSnapshotRegistry->copyFrom(
    //    m_registry->m_workerSnapshotRegistry,
    //    node->m_snapshotIndex, 1);

    // NOTE KI type must be prepared *before* node
    {
        event::Event evt { event::Type::type_prepare_view };
        evt.body.nodeType.target = node->m_typeHandle.toId();
        m_registry->m_dispatcherView->send(evt);
    }

    {
        event::Event evt { event::Type::node_added };
        evt.body.node.target = nodeId;
        m_registry->m_dispatcherWorker->send(evt);
    }

    KI_INFO(fmt::format("ATTACH_NODE: node={}", node->str()));
}

bool NodeRegistry::bindParent(
    const ki::node_id nodeId,
    const ki::node_id parentId)
{
    // NOTE KI everything else, except root requires parent
    if (nodeId == m_rootId) return true;

    auto* node = pool::NodeHandle::toNode(nodeId);
    auto* parent = pool::NodeHandle::toNode(parentId);

    if (!node || !parent) return false;

    assert(parent->m_entityIndex >= m_rootIndex);
    assert(parent->m_entityIndex < node->m_entityIndex);

    m_parentIndeces[node->m_entityIndex] = parent->m_entityIndex;

    KI_INFO(fmt::format("BIND_PARENT: parent={}, child={}", parentId, nodeId));

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

void NodeRegistry::setActiveCameraNode(pool::NodeHandle handle)
{
    auto* node = handle.toNode();
    if (!node) return;
    if (!node->m_camera) return;

    m_activeCameraNode = handle;
}

pool::NodeHandle NodeRegistry::getNextCameraNode(
    pool::NodeHandle srcNode,
    int offset) const noexcept
{
    int index = 0;
    int size = static_cast<int>(m_cameraNodes.size());
    for (int i = 0; i < size; i++) {
        if (m_cameraNodes[i] == srcNode) {
            index = std::max(0, (i + offset) % size);
            break;
        }
    }
    return m_cameraNodes[index];
}

pool::NodeHandle NodeRegistry::findDefaultCameraNode() const
{
    const auto& it = std::find_if(
        m_cameraNodes.begin(),
        m_cameraNodes.end(),
        [](pool::NodeHandle handle) {
            auto* node = handle.toNode();
            return node && node->m_camera->isDefault();
        });
    return it != m_cameraNodes.end() ? *it : NULL_HANDLE;
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

    {
        event::Event evt { event::Type::type_prepare_view };
        evt.body.nodeType.target = node->m_typeHandle.toId();
        m_registry->m_dispatcherView->send(evt);
    }
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
