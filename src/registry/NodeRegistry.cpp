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

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "component/Light.h"
#include "component/CameraComponent.h"

#include "particle/ParticleGenerator.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "generator/NodeGenerator.h"
#include "generator/TextGenerator.h"

#include "audio/Listener.h"
#include "audio/Source.h"
#include "audio/AudioEngine.h"

#include "physics/PhysicsEngine.h"
#include "script/ScriptEngine.h"

#include "EntitySSBO.h"

#include "Registry.h"
#include "MeshTypeRegistry.h"
#include "EntityRegistry.h"
#include "ModelRegistry.h"
#include "ControllerRegistry.h"
#include "registry/NodeSnapshotRegistry.h"

#include "registry/SnapshotRegistry_impl.h"

namespace {
    const std::vector<pool::NodeHandle> EMPTY_NODE_LIST;

    const ki::program_id NULL_PROGRAM_ID = 0;

    const pool::NodeHandle NULL_HANDLE = pool::NodeHandle::NULL_HANDLE;

    static NodeRegistry g_registry;
}

NodeRegistry& NodeRegistry::get() noexcept
{
    return g_registry;
}

NodeRegistry::NodeRegistry()
{
    // NOTE KI declare index == 0 as NULL object
    m_handles.emplace_back();
    m_parentIndeces.push_back(0);

    auto& nullState = m_states.emplace_back();
    // TODO KI cannot update due to thread check
    //nullState.updateRootMatrix();

    m_snapshotsWT.emplace_back();
    m_snapshotsPending.emplace_back();
    m_snapshotsRT.emplace_back();

    auto& nullEntity = m_entities.emplace_back();
    nullEntity.setModelMatrix(glm::mat4(1.f), true, true);
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

void NodeRegistry::prepare(
    Registry* registry)
{
    const auto& assets = Assets::get();

    m_registry = registry;
    m_selectionMaterial = std::make_unique<Material>();
    *m_selectionMaterial = Material::createMaterial(BasicMaterial::selection);
    m_selectionMaterial->registerMaterial();

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
        //std::lock_guard lock(m_lock);
        // NOTE KI nodes are in DAG order
        for (int i = 0; i < m_states.size(); i++) {
            auto* node = m_cachedNodesWT[i];
            if (!node) continue;

            auto& state = m_states[i];

            if (i == m_rootIndex) {
                state.updateRootMatrix();
            }
            else {
                auto& parentState = m_states[m_parentIndeces[i]];
                state.updateModelMatrix(parentState);
            }

            if (node->m_generator) {
                node->m_generator->updateWT(ctx, *node);
            }

            if (node->m_particleGenerator) {
                node->m_particleGenerator->updateWT(ctx, *node);
            }
        }
    }

    //{
    //    snapshotWT(*m_registry->m_workerSnapshotRegistry);
    //}

    //{
    //    std::lock_guard lock(m_lock);
    //    m_nodeLevel++;
    //}
}

void NodeRegistry::updateModelMatrices()
{
    {
        auto& state = m_states[0];
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

void NodeRegistry::snapshotWT()
{
    std::lock_guard lock(m_snapshotLock);

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
    snapshot(m_snapshotsWT, m_snapshotsPending);
}

void NodeRegistry::snapshotRT()
{
    snapshot(m_snapshotsPending, m_snapshotsRT);
    m_entities.resize(m_snapshotsRT.size());
}

void NodeRegistry::snapshot(
    std::vector<Snapshot>& src,
    std::vector<Snapshot>& dst)
{
    std::lock_guard lock(m_snapshotLock);

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

void NodeRegistry::updateEntity(const UpdateContext& ctx)
{
    //std::lock_guard lock(m_lock);

    for (int i = 0; i < m_snapshotsRT.size(); i++) {
        auto* node = m_cachedNodesRT[i];
        const auto& snapshot = m_snapshotsRT[i];

        if (!node || !snapshot.m_dirty) continue;

        auto& entity = m_entities[i];

        entity.u_objectID = node->getId();
        entity.u_highlightIndex = node->getHighlightIndex();

        auto* textGenerator = node->getGenerator<TextGenerator>();
        if (textGenerator) {
            entity.u_fontHandle = textGenerator->getAtlasTextureHandle();
        }

        snapshot.updateEntity(entity);
        snapshot.m_dirty = false;
    }
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
            attachNode(
                e.body.node.target,
                e.body.node.parentId,
                e.blob->body.state);
        });

    //dispatcher->addListener(
    //    event::Type::node_change_parent,
    //    [this](const event::Event& e) {
    //        changeParent(e.body.node.target, e.body.node.parentId);
    //    });

    dispatcher->addListener(
        event::Type::node_select,
        [this](const event::Event& e) {
            if (auto* node = pool::NodeHandle::toNode(e.body.node.target)) {
                node->setSelectionMaterialIndex(getSelectionMaterial().m_registeredIndex);
            }
        });

    dispatcher->addListener(
        event::Type::node_activate,
        [this](const event::Event& e) {
            setActiveNode(pool::NodeHandle::toHandle(e.body.node.target));
        });

    dispatcher->addListener(
        event::Type::camera_activate,
        [this](const event::Event& e) {
            auto handle = pool::NodeHandle::toHandle(e.body.node.target);
            if (!handle) handle = findDefaultCameraNode();
            setActiveCameraNode(handle);
        });

    dispatcher->addListener(
        event::Type::audio_listener_add,
        [this](const event::Event& e) {
            auto& data = e.blob->body.audioListener;
            auto* node = pool::NodeHandle::toNode(e.body.audioInit.target);
            if (!node) return;

            auto& ae = audio::AudioEngine::get();
            auto id = ae.registerListener();
            if (id) {
                node->m_audioListenerId = id;
                auto* listener = ae.getListener(id);

                listener->m_default = data.isDefault;
                listener->m_gain = data.gain;
                listener->m_nodeHandle = node->toHandle();
            }
        });

    dispatcher->addListener(
        event::Type::audio_source_add,
        [this](const event::Event& e) {
            auto& data = e.blob->body.audioSource;
            if (data.index < 0 || data.index >= ki::MAX_NODE_AUDIO_SOURCE) {
                return;
            }
            auto handle = pool::NodeHandle::toHandle(e.body.audioInit.target);
            auto& ae = audio::AudioEngine::get();
            auto id = ae.registerSource(data.soundId);
            if (id) {
                auto* node = handle.toNode();
                node->m_audioSourceIds[data.index] = id;

                auto* source = ae.getSource(id);

                source->m_autoPlay = data.isAutoPlay;
                source->m_referenceDistance = data.referenceDistance;
                source->m_maxDistance = data.maxDistance;
                source->m_rolloffFactor = data.rolloffFactor;
                source->m_minGain = data.minGain;
                source->m_maxGain = data.maxGain;
                source->m_looping = data.looping;
                source->m_gain = data.gain;
                source->m_pitch = data.pitch;
                source->m_nodeHandle = handle;
            }
        });

    dispatcher->addListener(
        event::Type::audio_listener_activate,
        [this](const event::Event& e) {
            auto& data = e.body.audioSource;
            audio::AudioEngine::get().setActiveListener(data.id);
        });

    dispatcher->addListener(
        event::Type::audio_source_play,
        [this](const event::Event& e) {
            auto& data = e.body.audioSource;
            audio::AudioEngine::get().playSource(data.id);
        });

    dispatcher->addListener(
        event::Type::audio_source_stop,
        [this](const event::Event& e) {
            auto& data = e.body.audioSource;
            audio::AudioEngine::get().stopSource(data.id);
        });

    dispatcher->addListener(
        event::Type::audio_source_pause,
        [this](const event::Event& e) {
            auto& data = e.body.audioSource;
            audio::AudioEngine::get().pauseSource(data.id);
        });

    if (assets.useScript) {
        dispatcher->addListener(
            event::Type::node_added,
            [this](const event::Event& e) {
                auto& data = e.body.node;
                auto handle = pool::NodeHandle::toHandle(data.target);
                const auto& scripts = script::ScriptEngine::get().getNodeScripts(handle);

                for (const auto& scriptId : scripts) {
                    {
                        event::Event evt { event::Type::script_run };
                        auto& body = evt.body.script = {
                            .target = data.target,
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
            auto& data = e.body.meshType;
            auto* type = pool::TypeHandle::toType(data.target);
            if (!type) return;
            type->prepareRT({ m_registry });
        });
}

void NodeRegistry::handleNodeAdded(Node* node)
{
    if (!node) return;

    auto handle = node->toHandle();
    auto* type = node->m_typeHandle.toType();

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

        if (light->m_directional) {
            m_dirLightNodes.push_back(handle);
        }
        else if (light->m_point) {
            m_pointLightNodes.push_back(handle);
        }
        else if (light->m_spot) {
            m_spotLightNodes.push_back(handle);
        }
    }
}

void NodeRegistry::selectNode(pool::NodeHandle handle, bool append) const noexcept
{
    if (!append) {
        for (auto* node : m_cachedNodesRT) {
            if (!node) continue;
            node->setSelectionMaterialIndex(-1);
        }
    }

    clearSelectedCount();

    auto* node = handle.toNode();
    if (!node) return;

    if (append && node->isSelected()) {
        KI_INFO(fmt::format("DESELECT: id={}", handle.str()));
        node->setSelectionMaterialIndex(-1);
    }
    else {
        KI_INFO(fmt::format("SELECT: id={}", handle.str()));
        node->setSelectionMaterialIndex(m_selectionMaterial->m_registeredIndex);
    }
}

void NodeRegistry::attachNode(
    const ki::node_id nodeId,
    const ki::node_id parentId,
    const NodeState& state) noexcept
{
    auto* node = pool::NodeHandle::toNode(nodeId);

    assert(node);
    assert(parentId || nodeId == m_rootId);

    if (nodeId != m_rootId) {
        assert(parentId >= m_rootIndex);
    }

    bindNode(nodeId, state);

    // NOTE KI ignore nodes without parent
    if (!bindParent(nodeId, parentId)) {
        KI_WARN_OUT(fmt::format("IGNORE: MISSING parent - parentId={}, node={}", parentId, node->str()));
        return;
    }

    auto* type = node->m_typeHandle.toType();

    type->prepareWT({ m_registry });
    node->prepareWT({ m_registry }, m_states[node->m_entityIndex]);

    if (type->m_flags.skybox) {
        return bindSkybox(node->toHandle());
    }
}

int NodeRegistry::countTagged() const noexcept
{
    ASSERT_RT();

    int count = m_taggedCount;
    if (count < 0) {
        count = 0;
        for (auto* node : m_cachedNodesRT) {
            if (node && node->isTagged()) count++;
        }
        m_taggedCount = count;
    }
    return count;
}

int NodeRegistry::countSelected() const noexcept
{
    ASSERT_RT();

    int count = m_selectedCount;
    if (count < 0) {
        count = 0;
        for (auto* node : m_cachedNodesRT) {
            if (node && node->isSelected()) count++;
        }
        m_selectedCount = count;
    }
    return count;
}

void NodeRegistry::changeParent(
    const ki::node_id nodeId,
    const ki::node_id parentId) noexcept
{
    bindParent(nodeId, parentId);
}

void NodeRegistry::bindNode(
    const ki::node_id nodeId,
    const NodeState& state)
{
    Node* node = pool::NodeHandle::toNode(nodeId);
    if (!node) return;

    pool::NodeHandle handle = node->toHandle();

    KI_INFO(fmt::format("BIND_NODE: {}", node->str()));

    auto* type = node->getType();

    node->m_entityIndex = static_cast<uint32_t>(m_handles.size());

    {
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

    clearSelectedCount();

    // NOTE KI ensure related snapshots are visible in RT
    // => otherwise IOOBE will trigger
    //m_registry->m_pendingSnapshotRegistry->copyFrom(
    //    m_registry->m_workerSnapshotRegistry,
    //    node->m_snapshotIndex, 1);

    // NOTE KI type must be prepared *before* node
    {
        event::Event evt { event::Type::type_prepare_view };
        evt.body.meshType.target = node->m_typeHandle.toId();
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

    assert(parent->m_entityIndex < node->m_entityIndex);

    m_parentIndeces[node->m_entityIndex] = parent->m_entityIndex;

    KI_INFO(fmt::format("BIND_PARENT: parent={}, child={}", parentId, nodeId));

    return true;
}

const Material& NodeRegistry::getSelectionMaterial() const noexcept
{
    return *m_selectionMaterial;
}

void NodeRegistry::setSelectionMaterial(const Material& material)
{
    *m_selectionMaterial = material;
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

    // NOTE KI active player is listener of audio
    // (regardless what camera is active)
    if (node->m_audioListenerId) {
        auto& ae = audio::AudioEngine::get();
        ae.setActiveListener(node->m_audioListenerId);
    }
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
        evt.body.meshType.target = node->m_typeHandle.toId();
        m_registry->m_dispatcherView->send(evt);
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
