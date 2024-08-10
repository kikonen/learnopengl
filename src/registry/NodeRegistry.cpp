#include "NodeRegistry.h"

#include <algorithm>

#include <glm/gtx/quaternion.hpp>

#include <fmt/format.h>

#include "util/thread.h"
#include "ki/limits.h"
#include "kigl/kigl.h"

#include "asset/Assets.h"
#include "asset/Program.h"

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

#include "Registry.h"
#include "MeshTypeRegistry.h"
#include "MaterialRegistry.h"
#include "EntityRegistry.h"
#include "ModelRegistry.h"
#include "ControllerRegistry.h"
#include "registry/NodeSnapshotRegistry.h"

#include "registry/SnapshotRegistry_impl.h"

namespace {
    const std::vector<pool::NodeHandle> EMPTY_NODE_LIST;

    const ki::program_id NULL_PROGRAM_ID = 0;

    const pool::NodeHandle NULL_HANDLE = pool::NodeHandle::NULL_HANDLE;

    static NodeRegistry s_registry;
}

NodeRegistry& NodeRegistry::get() noexcept
{
    return s_registry;
}

NodeRegistry::NodeRegistry()
{
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

    m_allNodes.clear();
    m_cachedNodesWT.clear();
    m_cachedNodesRT.clear();

    m_skybox.reset();

    pool::NodeHandle::clear();
}

void NodeRegistry::prepare(
    Registry* registry)
{
    m_registry = registry;
    m_selectionMaterial = std::make_unique<Material>();
    *m_selectionMaterial = Material::createMaterial(BasicMaterial::selection);
    MaterialRegistry::get().registerMaterial(*m_selectionMaterial);

    attachListeners();
}

void NodeRegistry::updateWT(const UpdateContext& ctx)
{
    if (m_cachedNodeLevel != m_nodeLevel) {
        ctx.m_registry->m_workerSnapshotRegistry->cacheNodes(m_cachedNodesWT);
        m_cachedNodeLevel = m_nodeLevel;
    }

    {
        //std::lock_guard lock(m_lock);
        // NOTE KI nodes are in DAG order
        for (auto* node : m_cachedNodesWT) {
            if (!node) continue;

            node->updateModelMatrix();

            if (node->m_generator) {
                node->m_generator->updateWT(ctx, *node);
            }

            if (node->m_particleGenerator) {
                node->m_particleGenerator->updateWT(ctx, *node);
            }
        }
    }

    physics::PhysicsEngine::get().updateBounds(ctx);
    ControllerRegistry::get().updateWT(ctx);

    {
        snapshotWT(*m_registry->m_workerSnapshotRegistry);
    }

    //{
    //    std::lock_guard lock(m_lock);
    //    m_nodeLevel++;
    //}
}

void NodeRegistry::snapshotWT(NodeSnapshotRegistry& snapshotRegistry)
{
    //std::lock_guard lock(m_lock);

    for (auto* node : m_cachedNodesWT) {
        if (!node) continue;

        const auto& state = node->getState();

        if (state.m_dirtySnapshot) {
            auto& snapshot = snapshotRegistry.modifySnapshot(node->m_snapshotIndex);
            snapshot.applyFrom(state);
        }

        if (node->m_generator) {
            node->m_generator->snapshotWT(snapshotRegistry);
        }
    }
}

void NodeRegistry::updateRT(const UpdateContext& ctx)
{
    ctx.m_registry->m_activeSnapshotRegistry->cacheNodes(m_cachedNodesRT);

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
    auto& snapshotRegistry = *ctx.m_registry->m_activeSnapshotRegistry;
    auto& entityRegistry = EntityRegistry::get();

    //std::lock_guard lock(m_lock);

    for (auto* node : m_cachedNodesRT) {
        if (!node) continue;
        if (node->m_entityIndex) {
            const auto& snapshot = snapshotRegistry.getSnapshot(node->m_snapshotIndex);
            if (snapshot.m_dirty) {
                auto* entity = entityRegistry.modifyEntity(node->m_entityIndex, true);

                entity->u_objectID = node->getId();
                entity->u_highlightIndex = node->getHighlightIndex();

                auto* textGenerator = node->getGenerator<TextGenerator>();
                if (textGenerator) {
                    entity->u_fontHandle = textGenerator->getAtlasTextureHandle();
                }

                snapshot.updateEntity(*entity);
                snapshot.m_dirty = false;
            }
        }

        if (node->m_generator) {
            node->m_generator->updateEntity(
                snapshotRegistry,
                entityRegistry,
                *node);
        }
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
                e.body.node.parentId);
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

                if (!scripts.empty()) {
                    KI_INFO_OUT(fmt::format("trigger_scripts: {}", scripts.size()));
                }

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

    if (type->hasMesh()) {
        node->m_entityIndex = EntityRegistry::get().registerEntity();
    }

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
    const ki::node_id parentId) noexcept
{
    const auto& assets = Assets::get();

    auto* node = pool::NodeHandle::toNode(nodeId);

    assert(node);
    assert(parentId || nodeId == assets.rootId);

    auto* type = node->m_typeHandle.toType();

    if (type->m_flags.skybox) {
        return bindSkybox(node->toHandle());
    }

    // NOTE KI ignore nodes without parent
    if (!bindParent(nodeId, parentId)) {
        KI_WARN_OUT(fmt::format("IGNORE: MISSING parent - parentId={}, node={}", parentId, node->str()));
        return;
    }

    bindNode(nodeId);
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
    auto* node = pool::NodeHandle::toNode(nodeId);
    auto* parent = pool::NodeHandle::toNode(parentId);

    if (!node) return;
    if (!parent) return;

    node->setParent(parent->toHandle());
}

void NodeRegistry::bindNode(
    const ki::node_id nodeId)
{
    Node* node = pool::NodeHandle::toNode(nodeId);
    if (!node) return;

    const auto& assets = Assets::get();

    pool::NodeHandle handle = node->toHandle();

    KI_INFO(fmt::format("BIND_NODE: {}", node->str()));

    auto* type = node->m_typeHandle.toType();

    type->prepareWT({ m_registry });
    node->prepareWT({ m_registry });

    {
        {
            //std::lock_guard lock(m_lock);
            m_allNodes.push_back(handle);
            m_nodeLevel++;
        }

        if (nodeId == assets.rootId) {
            m_rootWT = handle;
        }
    }

    clearSelectedCount();

    // NOTE KI ensure related snapshots are visible in RT
    // => otherwise IOOBE will trigger
    m_registry->m_pendingSnapshotRegistry->copyFrom(
        m_registry->m_workerSnapshotRegistry,
        node->m_snapshotIndex, 1);

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
    const auto& assets = Assets::get();

    // NOTE KI everything else, except root requires parent
    if (nodeId == assets.rootId) return true;

    auto* parent = pool::NodeHandle::toNode(parentId);
    auto* node = pool::NodeHandle::toNode(nodeId);

    if (!node) return false;

    if (!parent) {
        return false;
    }

    KI_INFO(fmt::format("BIND_PARENT: parent={}, child={}", parentId, nodeId));

    node->setParent(parent->toHandle());

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
    node->prepareWT({ m_registry });

    m_skybox = handle;

    {
        event::Event evt { event::Type::type_prepare_view };
        evt.body.meshType.target = node->m_typeHandle.toId();
        m_registry->m_dispatcherView->send(evt);
    }
}
