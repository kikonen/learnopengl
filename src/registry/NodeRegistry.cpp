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

#include "util/DagSort.h"
#include "util/DagSort_impl.h"

#include "mesh/LodMesh.h"
#include "mesh/LodMeshInstance.h"
#include "mesh/ModelMesh.h"

#include "component/definition/LightDefinition.h"
#include "component/definition/CameraComponentDefinition.h"
#include "component/definition/ParticleGeneratorDefinition.h"
#include "component/definition/GeneratorDefinition.h"
#include "component/definition/TextGeneratorDefinition.h"
#include "component/definition/AudioSourceDefinition.h"
#include "component/definition/AudioListenerDefinition.h"
#include "component/definition/PhysicsDefinition.h"
#include "component/definition/ControllerDefinition.h"

#include "particle/ParticleGenerator.h"

#include "engine/Engine.h"
#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "generator/NodeGenerator.h"

#include "animation/Rig.h"

#include "audio/Listener.h"
#include "audio/Source.h"
#include "audio/AudioSystem.h"

#include "physics/PhysicsSystem.h"
#include "physics/physics_util.h"

#include "component/Light.h"
#include "component/CameraComponent.h"

#include "controller/NodeController.h"

#include "generator/TextGenerator.h"

#include "debug/DebugContext.h"

#include "script/ScriptSystem.h"

#include "render/InstanceRegistry.h"

#include "EntitySSBO.h"

#include "Registry.h"
#include "NodeTypeRegistry.h"
#include "EntityRegistry.h"
#include "ControllerRegistry.h"

namespace {
    const std::vector<pool::NodeHandle> EMPTY_NODE_LIST;

    using t_dag_item = util::DagItem<pool::NodeHandle, model::Node>;

    constexpr int NULL_NODE_INDEX = 0;
    constexpr int ID_NODE_INDEX = 1;

    // NOTE KI null/id entityIndex is reserved in node pool
    constexpr int NULL_ENTITY_INDEX = 0;
    constexpr int ID_ENTITY_INDEX = 1;

    constexpr int INITIAL_SIZE = 30000;

    const ki::program_id NULL_PROGRAM_ID = 0;

    const pool::NodeHandle NULL_HANDLE = pool::NodeHandle::NULL_HANDLE;

    static NodeRegistry* s_registry{ nullptr };
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

        m_rootHandle.reset();
        m_rootRT.reset();
        m_rootWT.reset();
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
    pool::NodeHandle::clear();

    m_skybox.reset();

    m_rootHandle.reset();
    m_rootRT.reset();
    m_rootWT.reset();

    m_rootEntityIndex = 0;

    m_sortedNodes.clear();

    m_handles.clear();
    m_parentIndeces.clear();
    m_states.clear();
    m_snapshotBuffer.clear();
    m_entities.clear();
    m_dirtyEntities.clear();

    m_cachedNodesWT.clear();
    m_cachedNodesRT.clear();
    m_processedLevels.clear();

    m_nodeLevel = 0;
    m_cachedNodeLevelWT = 0;
    m_cachedNodeLevelRT = 0;

    m_activeNode.reset();

    {
        m_sortedNodes.reserve(INITIAL_SIZE);

        m_handles.reserve(INITIAL_SIZE);
        m_parentIndeces.reserve(INITIAL_SIZE);
        m_states.reserve(INITIAL_SIZE);
        m_snapshotBuffer.reserve(INITIAL_SIZE);
        m_entities.reserve(INITIAL_SIZE);
        m_dirtyEntities.reserve(INITIAL_SIZE);

        m_cachedNodesWT.reserve(INITIAL_SIZE);
        m_cachedNodesRT.reserve(INITIAL_SIZE);
        m_processedLevels.reserve(INITIAL_SIZE);
        m_processedNormalLevels.reserve(INITIAL_SIZE);

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
        m_parentIndeces.push_back(NULL_ENTITY_INDEX);
        m_sortedNodes.push_back(NULL_ENTITY_INDEX);
    }

    {
        // NOTE KI declare index == 1 as IDENTITY object
        auto& state = m_states.emplace_back();
        state.updateRootMatrix();

        m_handles.emplace_back();
        m_parentIndeces.push_back(NULL_ENTITY_INDEX);
        m_sortedNodes.push_back(ID_ENTITY_INDEX);
    }
}

void NodeRegistry::prepare(
    Engine* engine)
{
    const auto& assets = Assets::get();

    m_engine = engine;

    m_debug = assets.nodeRegistryDebug;
    m_deferSort = assets.nodeRegistryDeferSort;

    m_rootId = assets.rootId;

    {
        m_layerInfos.resize(assets.layers.size());
        for (int index = 0; auto& layerInfo : m_layerInfos) {
            layerInfo = { index++, glm::uvec2{1, 1} };
        }
    }

    attachListeners();
}

void NodeRegistry::updateWT(const UpdateContext& ctx)
{
    auto& cachedNodes = getCachedNodesWT();
    const auto& sortedNodes = getSortedNodes();

    {
        auto& physicsSystem = physics::PhysicsSystem::get();

        {
            auto& state = m_states[NULL_ENTITY_INDEX];
            state.updateRootMatrix();
        }
        {
            auto& state = m_states[ID_ENTITY_INDEX];
            state.updateRootMatrix();
        }
        {
            auto& state = m_states[m_rootEntityIndex];
            state.updateRootMatrix();
        }

        //std::lock_guard lock(m_lock);
        // NOTE KI nodes are in DAG order
        for (int sortedIndex = ID_NODE_INDEX + 1; sortedIndex < sortedNodes.size(); sortedIndex++)
        {
            auto entityIndex = sortedNodes[sortedIndex];

            // NOTE KI skip free/root slot
            if (m_parentIndeces[entityIndex] == 0) continue;

            auto* node = cachedNodes[entityIndex];
            if (!node) continue;

            auto& state = m_states[entityIndex];
            const auto& parentState = m_states[m_parentIndeces[entityIndex]];
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

    for (int entityIndex = ID_ENTITY_INDEX + 1; entityIndex < m_states.size(); entityIndex++) {
        // NOTE KI skip free/root slot
        if (m_parentIndeces[entityIndex] == 0) continue;

        auto* node = cachedNodes[entityIndex];
        if (!node) continue;

        auto& state = m_states[entityIndex];

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
    for (auto entityIndex = ID_ENTITY_INDEX + 1; entityIndex < m_states.size(); entityIndex++) {
        // NOTE KI skip free/root slot
        if (m_parentIndeces[entityIndex] == 0) continue;

        const auto parentLevel = m_states[m_parentIndeces[entityIndex]].m_matrixLevel;
        if (!m_states[entityIndex].valid(parentLevel)) return entityIndex;
    }
    return -1;
}

void NodeRegistry::updateModelMatrices()
{
    {
        auto& state = m_states[NULL_ENTITY_INDEX];
        state.updateRootMatrix();
    }
    {
        auto& state = m_states[ID_ENTITY_INDEX];
        state.updateRootMatrix();
    }
    {
        auto& state = m_states[m_rootEntityIndex];
        state.updateRootMatrix();
    }

    const auto& sortedNodes = getSortedNodes();
    for (int sortedIndex = ID_NODE_INDEX + 1; sortedIndex < sortedNodes.size(); sortedIndex++)
    {
        auto entityIndex = sortedNodes[sortedIndex];
        //for (auto i = m_rootIndex + 1; i < m_states.size(); i++) {
        // NOTE KI skip free/root slot
        if (m_parentIndeces[entityIndex] == 0) continue;

        m_states[entityIndex].updateModelMatrix(m_states[m_parentIndeces[entityIndex]]);
    }
}

void NodeRegistry::updateModelMatrices(const model::Node* node)
{
    auto index = node->getEntityIndex();
    m_states[index].updateModelMatrix(m_states[m_parentIndeces[index]]);
}

void NodeRegistry::publishSnapshots()
{
    m_snapshotBuffer.publish(m_states, m_parentIndeces);

    {
        auto& dbg = debug::DebugContext::modify();
        auto& physicsDbg = dbg.m_physics;

        physicsDbg.m_meshesPending.exchange(physicsDbg.m_meshesWT);

        std::shared_ptr<std::vector<mesh::MeshInstance>> tmp;
        physicsDbg.m_meshesWT.store(tmp);
    }
}

void NodeRegistry::syncSnapshots()
{
    // Swap RT's buffer with shared to get latest snapshots
    m_snapshotBuffer.sync();

    cacheNodes(m_cachedNodesRT, m_cachedNodeLevelRT);

    const auto sz = m_snapshotBuffer.size();
    m_entities.resize(sz);
    m_dirtyEntities.resize(sz);
    m_processedLevels.resize(sz, 0);
    m_processedNormalLevels.resize(sz, 0);

    auto& dbg = debug::DebugContext::modify();
    auto& physicsDbg = dbg.m_physics;

    if (physicsDbg.m_meshesPending.load()) {
        physicsDbg.m_meshesRT.exchange(physicsDbg.m_meshesPending);

        std::shared_ptr<std::vector<mesh::MeshInstance>> tmp;
        physicsDbg.m_meshesPending.store(tmp);
    }
}

void NodeRegistry::updateDrawables()
{
    auto& cachedNodes = getCachedNodesRT();
    const auto& snapshots = m_snapshotBuffer.getSnapshots();

    auto& instanceRegistry = render::InstanceRegistry::get();

    for (int entityIndex = ID_ENTITY_INDEX + 1; entityIndex < snapshots.size(); entityIndex++) {
        // NOTE KI skip free/root slot
        if (entityIndex >= m_parentIndeces.size() || m_parentIndeces[entityIndex] == 0) continue;

        const auto& snapshot = snapshots[entityIndex];

        // NOTE KI detect slot reuse: if snapshot level < processed level, slot was reused
        if (entityIndex < m_processedLevels.size() &&
            snapshot.m_matrixLevel < m_processedLevels[entityIndex]) {
            m_processedLevels[entityIndex] = 0;
            m_processedNormalLevels[entityIndex] = 0;
        }

        auto* node = cachedNodes[entityIndex];
        if (!node) continue;

        bool isLightweight = node->m_generator && node->m_generator->isLightWeight();

        // NOTE KI use level comparison instead of m_dirty flag
        if (!isLightweight) {
            if (entityIndex >= m_processedLevels.size() ||
                snapshot.m_matrixLevel <= m_processedLevels[entityIndex]) continue;
        }

        node->updateDrawables(instanceRegistry, snapshot);
    }
}

std::vector<model::Node*>& NodeRegistry::getCachedNodesWT()
{
    cacheNodes(m_cachedNodesWT, m_cachedNodeLevelWT);
    return m_cachedNodesWT;
}

void NodeRegistry::updateRT(const UpdateContext& ctx)
{
    m_rootRT = m_rootWT;

    auto* root = getRootRT();
    m_rootPreparedRT = root && root->m_preparedRT;

    updateDrawables();
}

std::pair<int, int> NodeRegistry::updateEntity(const UpdateContext& ctx)
{
    int minDirty = INT32_MAX;
    int maxDirty = INT32_MIN;

    const auto& snapshots = m_snapshotBuffer.getSnapshots();

    for (int entityIndex = 0; entityIndex < snapshots.size(); entityIndex++) {
        if (m_cachedNodesRT.size() < entityIndex + 1) continue;
        if (entityIndex >= m_processedLevels.size()) continue;

        auto* node = m_cachedNodesRT[entityIndex];
        const auto& state = m_states[entityIndex];
        const auto& snapshot = snapshots[entityIndex];

        // NOTE KI detect slot reuse: if snapshot level < processed level, slot was reused
        if (snapshot.m_matrixLevel < m_processedLevels[entityIndex]) {
            m_processedLevels[entityIndex] = 0;
            m_processedNormalLevels[entityIndex] = 0;
        }

        // NOTE KI use level comparison instead of m_dirty flag
        if (snapshot.m_matrixLevel <= m_processedLevels[entityIndex]) continue;

        auto& entity = m_entities[entityIndex];

        if (node) {
            entity.u_objectID = node->getId();

            auto* textGenerator = node->getGenerator<TextGenerator>();
            if (textGenerator) {
                entity.u_fontHandle = textGenerator->getAtlasTextureHandle();
            }
        }

        entity.u_tilingX = state.m_tilingX;
        entity.u_tilingY = state.m_tilingY;

        // NOTE KI dirtyNormal based on normalLevel comparison
        const bool dirtyNormal = snapshot.m_normalLevel > m_processedNormalLevels[entityIndex];
        snapshot.updateEntity(entity, dirtyNormal);

        // NOTE KI mark as processed for RT side tracking
        m_processedLevels[entityIndex] = snapshot.m_matrixLevel;
        m_processedNormalLevels[entityIndex] = snapshot.m_normalLevel;

        m_dirtyEntities[entityIndex] = true;
        if (entityIndex < minDirty) minDirty = entityIndex;
        if (entityIndex > maxDirty) maxDirty = entityIndex;
    }

    return { minDirty, maxDirty };
}

void NodeRegistry::cacheNodes(
    std::vector<model::Node*>& cache,
    ki::level_id& cacheLevel)
{
    if (cacheLevel == m_nodeLevel) return;
    cacheLevel = m_nodeLevel;

    cache.resize(m_handles.size());
    cache[NULL_ENTITY_INDEX] = nullptr;
    cache[ID_ENTITY_INDEX] = nullptr;

    for (size_t i = ID_ENTITY_INDEX + 1; i < m_handles.size(); i++) {
		if (!m_handles[i]) continue;
        cache[i] = m_handles[i].toNode();
    }
}

void NodeRegistry::attachListeners()
{
    const auto& assets = Assets::get();
    auto* registry = m_engine->getRegistry();

    {
        auto* dispatcher = registry->m_dispatcherWorker;

        m_listen_node_add.listen(
            event::Type::node_add,
            dispatcher,
            [this](const event::Event& e) {
                const auto& data = e.body.node;

                attachNode(
                    pool::NodeHandle::toHandle(data.target),
                    pool::NodeHandle::toHandle(data.parentId),
                    data.socketId,
                    e.attachment->nodeEntry.state);
            });

        m_listen_node_remove.listen(
            event::Type::node_remove,
            dispatcher,
            [this](const event::Event& e) {
                detachNode(
                    pool::NodeHandle::toHandle(e.body.node.target));
            });

        m_listen_node_dispose.listen(
            event::Type::node_dispose,
            dispatcher,
            [this](const event::Event& e) {
                disposeNode(
                    pool::NodeHandle::toHandle(e.body.node.target));
            });

        m_listen_node_activate.listen(
            event::Type::node_activate,
            dispatcher,
            [this](const event::Event& e) {
                setActiveNode(pool::NodeHandle::toHandle(e.body.node.target));
            });

        if (assets.useScript) {
            m_listen_node_added.listen(
                event::Type::node_added,
                dispatcher,
                [this, registry](const event::Event& e) {
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
                        }
                        else
                        {
                            event::Event evt{ event::Type::script_run };
                            auto& body = evt.body.script = {
                                .target = nodeId,
                                .id = scriptId,
                            };
                            registry->m_dispatcherWorker->send(evt);
                        }
                    }
                });
        }

        m_listen_viewport_changed.listen(
            event::Type::viewport_changed,
            dispatcher,
            [this](const event::Event& e) {
                auto& data = e.body.view;
                viewportChanged(
                    data.layer,
                    data.aspectRatio);
            });
    }

    {
        auto* dispatcherView = registry->m_dispatcherView;

        m_listen_type_prepare_view.listen(
            event::Type::type_prepare_view,
            dispatcherView,
            [this](const event::Event& e) {
                auto& data = e.body.nodeType;
                auto* type = pool::TypeHandle::toType(data.target);
                if (!type) return;
                type->prepareRT({ *m_engine });
            });
    }
}

void NodeRegistry::handleNodeAdded(model::Node* node)
{
    if (!node) return;

    // NOTE KI use getSnapshotLatest to handle race condition
    // when event arrives before atomic read index is visible to RT
    const auto* snapshot = m_snapshotBuffer.getSnapshotLatest(node->getEntityIndex());
    if (!snapshot) {
        KI_CRITICAL(fmt::format("handleNodeAdded: missing snapshot - node={}", node->str()));
        return;
    }

    auto nodeHandle = node->toHandle();

    node->prepareRT({ *m_engine }, *snapshot);

    if (node->m_generator) {
        const PrepareContext ctx{ *m_engine };
        node->m_generator->prepareRT(ctx, *node, *snapshot);
    }

    node->registerDrawables(
        render::InstanceRegistry::get(),
        *snapshot);

    node->m_preparedRT = true;
}

void NodeRegistry::handleNodeRemoved(model::Node* node)
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

    auto* registry = m_engine->getRegistry();

    for (auto& nodeHandle : m_pendingAdded) {
        auto* node = nodeHandle.toNode();
        if (!node) continue;

        {
            event::Event evt{ event::Type::node_added };
            evt.body.node.target = nodeHandle;
            registry->m_dispatcherWorker->send(evt);
        }

        // NOTE KI type must be prepared *before* node
        {
            event::Event evt{ event::Type::type_prepare_view };
            evt.body.nodeType.target = node->m_typeHandle.toId();
            registry->m_dispatcherView->send(evt);
        }

        {
            event::Event evt{ event::Type::node_added };
            evt.body.node.target = nodeHandle;
            registry->m_dispatcherView->send(evt);
        }

        if (node->m_typeFlags.skybox)
        {
            event::Event evt{ event::Type::type_prepare_view };
            evt.body.nodeType.target = node->m_typeHandle.toId();
            registry->m_dispatcherView->send(evt);
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
            registry->m_dispatcherView->send(evt);
        }

        {
            event::Event evt{ event::Type::node_removed };
            evt.body.node.target = nodeHandle;
            registry->m_dispatcherWorker->send(evt);
        }

        {
            event::Event evt{ event::Type::node_removed };
            evt.body.node.target = nodeHandle;
            registry->m_dispatcherView->send(evt);
        }
    }

    m_pendingRemoved.clear();
}

bool NodeRegistry::isPendingRemoveConflict(uint32_t entityIndex)
{
    const auto& it = std::find_if(
        m_pendingRemoved.cbegin(),
        m_pendingRemoved.cend(),
        [entityIndex](const auto& handle) { return handle.m_handleIndex == entityIndex; });
    return it != m_pendingRemoved.end();
}

void NodeRegistry::attachNode(
    const pool::NodeHandle nodeHandle,
    ki::node_id parentId,
    ki::socket_id socketId,
    const model::CreateState& state) noexcept
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

    if (isPendingRemoveConflict(node->getEntityIndex())) {
        KI_CRITICAL(fmt::format(
            "IGNORE: ENTITY_CONFLICT node={}",
            node->str()));
        return;
    }

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

    sortNodes(nodeHandle, true, false);

    bindParentSocket(nodeHandle, socketId);

    auto* type = node->m_typeHandle.toType();

    type->prepareWT({ *m_engine });
    node->prepareWT({ *m_engine }, m_states[node->getEntityIndex()]);

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
            node->getEntityIndex(),
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
            auto controller = ControllerDefinition::createController(definition);
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

    node->m_alive = false;

    if (!unbindParent(nodeHandle)) {
        KI_WARN_OUT(fmt::format(
            "ERROR: parent unbind failed - node={}",
            node->str()));
        return;
    }

    node->unprepareWT({ *m_engine }, m_states[node->getEntityIndex()]);

    const auto* type = node->m_typeHandle.toType();

    if (node->m_physicsObjectId)
    {
        auto& physicsSystem = physics::PhysicsSystem::get();
        physicsSystem.unregisterObject(node->m_physicsObjectId);
    }

    unbindNode(nodeHandle);

    sortNodes(nodeHandle, false, true);

    //node->unprepareWT({ m_registry }, m_states[node->m_entityIndex]);
}

void NodeRegistry::disposeNode(
    const pool::NodeHandle nodeHandle) noexcept
{
    uint32_t entityIndex = 0;
    {
        auto* node = nodeHandle.toNode();
        if (!node) return;

        entityIndex = entityIndex = node->getEntityIndex();
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
    //m_freeIndeces.push_back(entityIndex);
}

void NodeRegistry::changeParent(
    const pool::NodeHandle nodeHandle,
    ki::node_id parentId,
    ki::socket_id socketId) noexcept
{
    bindParent(nodeHandle, parentId);
    // TODO KI need to save socketId
    bindParentSocket(nodeHandle, 0);
}

void NodeRegistry::sortNodes(
	const pool::NodeHandle targetHandle,
	bool add,
	bool remove)
{
    auto& sortedNodes = m_sortedNodes;
    //sortedNodes.reserve(m_handles.size());
    {
	    auto entityIndex = targetHandle.toIndex();
	    if (add) {
		    sortedNodes.push_back(entityIndex);
	    }
	    if (remove) {
		    // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
		    const auto& it = std::remove_if(
			    sortedNodes.begin(),
			    sortedNodes.end(),
			    [&entityIndex](auto& e) {
				    return e == entityIndex;
			    });
		    sortedNodes.erase(it, sortedNodes.end());
	    }
    }

    // Defer expensive DAG sort during batch loading
    if (m_deferSort) {
	    m_sortDirty = true;
	    return;
    }

    performDagSort();
}

void NodeRegistry::flushDeferredSort()
{
	if (!m_sortDirty) return;
	m_sortDirty = false;
	performDagSort();
}

const std::vector<uint32_t>& NodeRegistry::getSortedNodes()
{
	flushDeferredSort();
	return m_sortedNodes;
}

void NodeRegistry::performDagSort()
{
	std::vector<t_dag_item> sorted;
	{
		std::vector<t_dag_item> items;
		items.reserve(m_sortedNodes.size() - ID_NODE_INDEX + 1);

		for (int sortedIndex = ID_NODE_INDEX + 1; sortedIndex < m_sortedNodes.size(); sortedIndex++)
		{
			auto entityIndex = m_sortedNodes[sortedIndex];
			const auto parentEntityIndex = m_parentIndeces[entityIndex];

			const auto nodeHandle = m_handles[entityIndex];
			const auto parentHandle = m_handles[parentEntityIndex];

			items.push_back({ parentHandle, nodeHandle, nullptr });
		}

		util::DagSort<pool::NodeHandle, model::Node> sorter;
		sorted = sorter.sort(items);
	}

	for (auto i = 0; i < sorted.size(); i++) {
		auto entityIndex = sorted[i].nodeId.toIndex();
		m_sortedNodes[i + ID_NODE_INDEX + 1] = entityIndex;
	}
}

void NodeRegistry::bindNode(
    const pool::NodeHandle nodeHandle,
    const model::CreateState& createState)
{
    auto* node = nodeHandle.toNode();
    if (!node) return;

    if (m_debug) KI_INFO(fmt::format("BIND_NODE: {}", node->str()));

    uint32_t entityIndex = node->getEntityIndex();
    bool reuse = false;

    //// NOTE KI reuse slots (after dispose)
    //if (!m_freeIndeces.empty()) {
    //    entityIndex = m_freeIndeces[m_freeIndeces.size() - 1];
    //    m_freeIndeces.pop_back();
    //    reuse = true;
    //}
    //else {
    //    entityIndex = static_cast<uint32_t>(m_handles.size());
    //}

    {
        const auto* type = node->getType();

        node->m_typeFlags = type->m_flags;
        node->m_layer = type->m_layer;

        node->m_camera = CameraComponentDefinition::createCameraComponent(type);
        node->m_light = LightDefinition::createLight(type);
        node->m_particleGenerator = ParticleGeneratorDefinition::createParticleGenerator(type);
        node->m_generator = GeneratorDefinition::createGenerator(type);
        node->m_audioSources = AudioSourceDefinition::createAudioSources(type);
        node->m_audioListener = AudioListenerDefinition::createAudioListener(type);
        if (node->m_typeFlags.text) {
            node->m_generator = TextGeneratorDefinition::createTextGenerator(type);
        }
    }

    {
        model::NodeState state;
        {
            const auto* type = node->getType();

            state.setBaseScale(type->m_baseScale);
            state.setPosition(createState.m_position);
            state.setScale(createState.m_scale);
            state.setRotation(createState.m_rotation);
            state.m_tilingX = createState.m_tilingX;
            state.m_tilingY = createState.m_tilingY;
            state.m_tagId = createState.m_tagId;
        }

        if (!reuse) {
            auto newSize = std::max(static_cast<size_t>(entityIndex + 1), m_handles.size());
            m_handles.resize(newSize);
            m_parentIndeces.resize(newSize);
            m_states.resize(newSize);
        }

        m_handles[entityIndex] = nodeHandle;
        m_parentIndeces[entityIndex] = 0;
        m_states[entityIndex] = state;

        if (node->m_layer < m_layerInfos.size()) {
            m_states[entityIndex].setAspectRatio(m_layerInfos[node->m_layer].m_aspectRatio);
        }

        m_nodeLevel++;

        if (nodeHandle == m_rootHandle) {
            assert(!m_rootWT);
            m_rootWT = nodeHandle;
            m_rootEntityIndex = node->getEntityIndex();
        }
    }

    if (m_debug) KI_INFO(fmt::format("ATTACH_NODE: node={}", node->str()));
}

void NodeRegistry::unbindNode(
    const pool::NodeHandle nodeHandle)
{
    auto* node = nodeHandle.toNode();
    if (!node) return;

    if (m_debug) KI_INFO(fmt::format("UNBIND_NODE: {}", node->str()));

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
        const auto entityIndex = node->getEntityIndex();

        m_handles[entityIndex] = pool::NodeHandle::NULL_HANDLE;
        // NOTE KI setting parentIndeces to 0 causes publish() to skip this slot
        m_parentIndeces[entityIndex] = 0;
        {
            m_states[entityIndex] = {};
            auto& state = m_states[entityIndex];
            state.m_dirty = false;
        }
        if (entityIndex < m_entities.size()) {
            m_entities[entityIndex] = {};
            m_dirtyEntities[entityIndex] = false;
        }
        if (entityIndex < m_cachedNodesWT.size()) {
            m_cachedNodesWT[entityIndex] = nullptr;
        }
        if (entityIndex < m_cachedNodesRT.size()) {
            m_cachedNodesRT[entityIndex] = nullptr;
        }

        m_nodeLevel++;
    }

    m_pendingRemoved.push_back(nodeHandle);
}

bool NodeRegistry::bindParent(
    const pool::NodeHandle nodeHandle,
    ki::node_id parentId)
{
    // NOTE KI everything else, except root requires parent
    if (!nodeHandle || nodeHandle == m_rootHandle) return true;

    auto parentHandle = pool::NodeHandle::toHandle(parentId);

    auto* node = nodeHandle.toNode();
    auto* parent = parentHandle.toNode();

    if (!node || !parent) {
        KI_INFO(fmt::format(
            "PARENT_MISSING: parent={}, child={}",
            parentHandle.str(),
            nodeHandle.str()));
        return false;
    }

    m_parentIndeces[node->getEntityIndex()] = parent->getEntityIndex();

    if (m_debug) KI_INFO(fmt::format(
        "BIND_PARENT: parent={}, child={}",
        parentHandle.str(),
        nodeHandle.str()));

    return true;
}

bool NodeRegistry::unbindParent(
    const pool::NodeHandle nodeHandle)
{
    if (nodeHandle == m_rootHandle) return true;

    auto* node = nodeHandle.toNode();
    if (!node) return false;

    auto parentIndex = m_parentIndeces[node->getEntityIndex()];
    auto parentHandle = m_handles[parentIndex];

    m_parentIndeces[node->getEntityIndex()] = 0;

    if (m_debug) KI_INFO(fmt::format(
        "UNBIND_PARENT: parent={}, child={}",
        parentHandle.str(),
        nodeHandle.str()));

    return true;
}

bool NodeRegistry::bindParentSocket(
    const pool::NodeHandle nodeHandle,
    ki::socket_id socketId)
{
    if (!socketId) return true;

    const auto& socketName = SID_NAME(socketId);

    auto* node = nodeHandle.toNode();
    auto* parent = node->getParent();

    if (!parent) {
        auto parentId = m_parentIndeces[node->getEntityIndex()];
        KI_INFO_OUT(fmt::format(
            "PARENT_BIND_ERROR: parent_missing - parent={}/{}, socket={}, child={}",
            parentId,
            SID_NAME(parentId),
            socketName,
            nodeHandle.str()));
        return false;
    }

    const auto* parentType = parent->getType();
    const auto& parentState = m_states[parent->getEntityIndex()];
    auto& state = m_states[node->getEntityIndex()];

    bool found = false;
    for (int index = -1; const auto& lodMesh : parentType->getLodMeshes()) {
        index++;

        auto* modelMesh = lodMesh.getMesh<mesh::ModelMesh>();
        if (!modelMesh) continue;

        auto* rig = modelMesh->getRig();
        if (!rig) continue;

        const auto* socket = rig->findSocket(socketName);

        if (socket) {
            const auto& lod = parent->getLodMeshInstances()[index];
            state.m_attachedSocketIndex = lod.m_socketBaseIndex + socket->m_index;
            found = true;
        }
    }

    if (!found) {
        KI_INFO_OUT(fmt::format(
            "PARENT_BIND_ERROR: rig_missing or socket_missing - parent={}, socket={}",
            parent->str(),
            socketName));
        return false;
    }

    return true;
}

//void NodeRegistry::withLock(const std::function<void(NodeRegistry&)>& fn)
//{
//    std::lock_guard lock(m_lock);
//    fn(*this);
//}

pool::NodeHandle NodeRegistry::findTagged(
    ki::tag_id tagId)
{
    for (int entityIndex = 0; entityIndex < m_states.size(); entityIndex++) {
        const auto& state = m_states[entityIndex];
        if (state.m_tagId == tagId) {
            return m_handles[entityIndex];
        }
    }

    return pool::NodeHandle::NULL_HANDLE;
}

std::vector<pool::NodeHandle> NodeRegistry::findTaggedAll(
    ki::tag_id tagId)
{
    std::vector<pool::NodeHandle> result;

    for (int entityIndex = 0; entityIndex < m_states.size(); entityIndex++) {
        const auto& state = m_states[entityIndex];
        if (state.m_tagId == tagId) {
            result.push_back(m_handles[entityIndex]);
        }
    }

    return result;
}

pool::NodeHandle NodeRegistry::findTaggedChild(
    pool::NodeHandle handle,
    ki::tag_id tagId)
{
    const auto* node = handle.toNode();
    if (!node) return pool::NodeHandle::NULL_HANDLE;
    const auto parentIndex = node->getEntityIndex();

    for (int childEntityIndex = 0; childEntityIndex < m_parentIndeces.size(); childEntityIndex++) {
        if (m_parentIndeces[childEntityIndex] == parentIndex) {
            const auto& state = m_states[childEntityIndex];
            if (state.m_tagId == tagId) {
                return m_handles[childEntityIndex];
            }
        }
    }

    return pool::NodeHandle::NULL_HANDLE;
}

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

    type->prepareWT({ *m_engine });
    node->prepareWT({ *m_engine }, m_states[node->getEntityIndex()]);

    m_skybox = handle;
}

void NodeRegistry::viewportChanged(
    uint8_t layer,
    glm::uvec2 aspectRatio) noexcept
{
    if (layer >= m_layerInfos.size()) return;

    auto& info = m_layerInfos[layer];
    info.m_aspectRatio = aspectRatio;

    const auto& sortedNodes = getSortedNodes();
    for (int sortedIndex = ID_NODE_INDEX + 1; sortedIndex < sortedNodes.size(); sortedIndex++)
    {
        auto entityIndex = sortedNodes[sortedIndex];
        auto& state = m_states[entityIndex];

        auto* node = getCachedNodesWT()[entityIndex];
        if (!node) continue;

        if (node->m_layer == info.m_index) {
            state.setAspectRatio(info.m_aspectRatio);
        }
    }
}

void NodeRegistry::updateBounds(
    const UpdateContext& ctx,
    model::NodeState& state,
    const model::NodeState& parentState,
    const model::Node* node,
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
