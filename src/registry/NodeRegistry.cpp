#include "NodeRegistry.h"

#include <algorithm>

#include <glm/gtx/quaternion.hpp>

#include <fmt/format.h>

#include "ki/limits.h"

#include "kigl/kigl.h"

#include "asset/Program.h"

#include "model/Node.h"

#include "mesh/MeshType.h"

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "generator/NodeGenerator.h"

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
#include "SnapshotRegistry.h"

namespace {
    const NodeVector EMPTY_NODE_LIST;

    const ki::program_id NULL_PROGRAM_ID = 0;
}

NodeRegistry::NodeRegistry(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_assets(assets),
    m_alive(alive)
{
}

NodeRegistry::~NodeRegistry()
{
    std::lock_guard<std::mutex> lock(m_snapshotLock);

    // NOTE KI forbid access into deleted nodes
    {
        m_idToNode.clear();
        m_uuidToNode.clear();

        //m_parentToChildren.clear();
    }

    {
        m_activeNode = nullptr;
        m_activeCameraNode = nullptr;
        m_cameraNodes.clear();

        m_dirLightNodes.clear();
        m_pointLightNodes.clear();
        m_spotLightNodes.clear();

        m_root = nullptr;
    }

    for (auto* node : m_allNodes) {
        delete node;
    }
    m_allNodes.clear();

    for (auto& [parentId, nodes] : m_pendingChildren) {
        for (auto& [uuid, node] : nodes) {
            delete node;
        }
    }
    m_pendingChildren.clear();

    if (m_skybox) {
        delete m_skybox;
    }
}

void NodeRegistry::prepare(
    Registry* registry)
{
    m_registry = registry;
    m_selectionMaterial = std::make_unique<Material>();
    *m_selectionMaterial = Material::createMaterial(BasicMaterial::selection);
    registry->m_materialRegistry->registerMaterial(*m_selectionMaterial);

    attachListeners();
}

void NodeRegistry::updateWT(const UpdateContext& ctx)
{
    //std::lock_guard<std::mutex> lock(m_snapshotLock);

    if (m_root) {
        m_root->updateWT(ctx);
    }

    ctx.m_registry->m_physicsEngine->updateBounds(ctx);
    ctx.m_registry->m_controllerRegistry->updateWT(ctx);

    {
        m_registry->withLock([this](auto& registry) {
            snapshotWT(*registry.m_snapshotRegistry);
        });
    }
}

void NodeRegistry::snapshotWT(SnapshotRegistry& snapshotRegistry)
{
    std::lock_guard<std::mutex> lock(m_snapshotLock);

    for (auto* node : m_allNodes) {
        auto& transform = node->modifyTransform();

        if (transform.m_dirtySnapshot) {
            auto& snapshot = snapshotRegistry.modifySnapshot(node->m_snapshotIndex);
            snapshot = transform;
            snapshot.m_dirty = true;
            transform.m_dirtySnapshot = false;
        }

        if (node->m_generator) {
            node->m_generator->snapshotWT(snapshotRegistry);
        }
    }
}

void NodeRegistry::snapshotRT(SnapshotRegistry& snapshotRegistry)
{}

void NodeRegistry::updateRT(const UpdateContext& ctx)
{
    m_rootPreparedRT = m_root && m_root->m_preparedRT;

    for (auto& node : m_cameraNodes) {
        node->m_camera->updateRT(ctx, *node);
    }
    for (auto& node : m_pointLightNodes) {
        node->m_light->updateRT(ctx, *node);
    }
    for (auto& node : m_spotLightNodes) {
        node->m_light->updateRT(ctx, *node);
    }
    for (auto& node : m_dirLightNodes) {
        node->m_light->updateRT(ctx, *node);
    }
}

void NodeRegistry::updateEntity(const UpdateContext& ctx)
{
    auto& snapshotRegistry = *ctx.m_registry->m_snapshotRegistry;
    auto& entityRegistry = *ctx.m_registry->m_entityRegistry;

    std::lock_guard<std::mutex> lock(m_snapshotLock);

    for (auto* node : m_allNodes) {
        if (node->m_snapshotIndex) {
            auto& snapshot = snapshotRegistry.modifySnapshot(node->m_snapshotIndex);
            if (snapshot.m_dirty) {
                auto* entity = entityRegistry.modifyEntity(node->m_entityIndex, true);

                entity->u_objectID = node->m_id;
                entity->u_highlightIndex = node->getHighlightIndex(ctx.m_assets);
                snapshot.updateEntity(*entity);
                snapshot.m_dirty = false;
            }
        }

        if (node->m_generator) {
            node->m_generator->updateEntity(
                ctx.m_assets,
                snapshotRegistry,
                entityRegistry,
                *node);
        }
    }
}

void NodeRegistry::attachListeners()
{
    auto* dispatcher = m_registry->m_dispatcher;
    auto* dispatcherView = m_registry->m_dispatcherView;

    dispatcher->addListener(
        event::Type::node_add,
        [this](const event::Event& e) {
            attachNode(
                e.body.node.target,
                e.body.node.uuid,
                e.body.node.parentUUID,
                e.body.node.parentId);
        });

    if (m_assets.useScript) {
        dispatcher->addListener(
            event::Type::node_added,
            [this](const event::Event& e) {
                auto& data = e.body.node;
                auto* node = data.target;

                auto* se = m_registry->m_scriptEngine;
                const auto& scripts = se->getNodeScripts(node->m_id);

                for (const auto& scriptId : scripts) {
                    {
                        event::Event evt { event::Type::script_run };
                        auto& body = evt.body.script = {
                            .target = node->m_id,
                            .id = scriptId,
                        };
                        m_registry->m_dispatcher->send(evt);
                    }
                }
            });
    }

    //dispatcher->addListener(
    //    event::Type::node_change_parent,
    //    [this](const event::Event& e) {
    //        changeParent(e.body.node.target, e.body.node.parentId);
    //    });

    dispatcher->addListener(
        event::Type::node_select,
        [this](const event::Event& e) {
            auto& node = e.body.node.target;
            node->setSelectionMaterialIndex(getSelectionMaterial().m_registeredIndex);
        });

    dispatcher->addListener(
        event::Type::node_activate,
        [this](const event::Event& e) {
            auto* node = e.body.node.target;
            setActiveNode(e.body.node.target);
        });

    dispatcher->addListener(
        event::Type::camera_activate,
        [this](const event::Event& e) {
            auto* node = e.body.node.target;
            if (!node) node = findDefaultCameraNode();
            setActiveCameraNode(node);
        });

    dispatcher->addListener(
        event::Type::audio_listener_add,
        [this](const event::Event& e) {
            auto& data = e.blob->body.audioListener;
            auto* node = getNode(e.body.audioInit.target);
            auto* ae = m_registry->m_audioEngine;
            auto id = ae->registerListener();
            if (id) {
                auto* listener = ae->getListener(id);

                listener->m_default = data.isDefault;
                listener->m_gain = data.gain;
                listener->m_node = node;
            }
        });

    dispatcher->addListener(
        event::Type::audio_source_add,
        [this](const event::Event& e) {
            auto& data = e.blob->body.audioSource;
            if (data.index < 0 || data.index >= ki::MAX_NODE_AUDIO_SOURCE) {
                return;
            }
            auto* node = getNode(e.body.audioInit.target);
            auto* ae = m_registry->m_audioEngine;
            auto id = ae->registerSource(data.soundId);
            if (id) {
                node->m_audioSourceIds[data.index] = id;

                auto* source = ae->getSource(id);

                source->m_autoPlay = data.isAutoPlay;
                source->m_referenceDistance = data.referenceDistance;
                source->m_maxDistance = data.maxDistance;
                source->m_rolloffFactor = data.rolloffFactor;
                source->m_minGain = data.minGain;
                source->m_maxGain = data.maxGain;
                source->m_looping = data.looping;
                source->m_gain = data.gain;
                source->m_pitch = data.pitch;
                source->m_node = node;
            }
        });

    dispatcher->addListener(
        event::Type::audio_listener_activate,
        [this](const event::Event& e) {
            auto& data = e.body.audioSource;
            m_registry->m_audioEngine->setActiveListener(data.id);
        });

    dispatcher->addListener(
        event::Type::audio_source_play,
        [this](const event::Event& e) {
            auto& data = e.body.audioSource;
            m_registry->m_audioEngine->playSource(data.id);
        });

    dispatcher->addListener(
        event::Type::audio_source_stop,
        [this](const event::Event& e) {
            auto& data = e.body.audioSource;
            m_registry->m_audioEngine->stopSource(data.id);
        });

    dispatcher->addListener(
        event::Type::audio_source_pause,
        [this](const event::Event& e) {
            auto& data = e.body.audioSource;
            m_registry->m_audioEngine->pauseSource(data.id);
        });

    dispatcher->addListener(
        event::Type::physics_add,
        [this](const event::Event& e) {
            auto& data = e.blob->body.physics;
            auto* pe = m_registry->m_physicsEngine;
            auto* node = getNode(e.body.physics.target);

            auto id = pe->registerObject();

            if (id) {
                auto* obj = pe->getObject(id);
                obj->m_update = data.update;
                obj->m_body = data.body;
                obj->m_geom = data.geom;
                obj->m_node = node;
            }
        });

    if (m_assets.useScript) {
        dispatcher->addListener(
            event::Type::script_bind,
            [this](const event::Event& e) {
                auto& data = e.body.script;
                auto* node = getNode(data.target);
                m_registry->m_scriptEngine->bindNodeScript(data.target, data.id);
            });

        dispatcher->addListener(
            event::Type::script_run,
            [this](const event::Event& e) {
                auto& data = e.body.script;
                if (data.target) {
                    auto* node = getNode(data.target);
                    m_registry->m_scriptEngine->runNodeScript(node, data.id);
                }
                else {
                    m_registry->m_scriptEngine->runGlobalScript(data.id);
                }
            });
    }

    dispatcherView->addListener(
        event::Type::type_prepare_view,
        [this](const event::Event& e) {
            auto& data = e.body.meshType;
            auto* type = m_registry->m_typeRegistry->modifyType(data.target);
            type->prepareRT({ m_assets, m_registry });
        });
}

void NodeRegistry::handleNodeAdded(Node* node)
{
    m_registry->m_snapshotRegistry->copyFromPending(0);

    node->m_preparedRT = true;

    if (node->m_type->getMesh()) {
        node->m_entityIndex = m_registry->m_entityRegistry->registerEntity();
    }

    if (node->m_camera) {
        m_cameraNodes.push_back(node);
        if (!m_activeCameraNode && node->m_camera->isDefault()) {
            m_activeCameraNode = node;
        }
    }

    if (node->m_light) {
        Light* light = node->m_light.get();

        if (light->m_directional) {
            m_dirLightNodes.push_back(node);
        }
        else if (light->m_point) {
            m_pointLightNodes.push_back(node);
        }
        else if (light->m_spot) {
            m_spotLightNodes.push_back(node);
        }
    }
}

void NodeRegistry::selectNodeById(ki::node_id id, bool append) const noexcept
{
    if (!append) {
        for (auto& x : m_idToNode) {
            x.second->setSelectionMaterialIndex(-1);
        }
    }

    clearSelectedCount();

    Node* node = getNode(id);
    if (!node) return;

    if (append && node->isSelected()) {
        KI_INFO(fmt::format("DESELECT: id={}", id));
        node->setSelectionMaterialIndex(-1);
    }
    else {
        KI_INFO(fmt::format("SELECT: id={}", id));
        node->setSelectionMaterialIndex(m_selectionMaterial->m_registeredIndex);
    }
}

void NodeRegistry::attachNode(
    Node* node,
    const uuids::uuid& uuid,
    const uuids::uuid& parentUUID,
    ki::node_id parentId) noexcept
{
    m_idToNode.insert(std::make_pair(node->m_id, node));

    if (node->m_type->m_flags.skybox) {
        return bindSkybox(node);
    }

    // NOTE KI ignore children without parent; until parent is found
    if (!bindParent(node, uuid, parentUUID, parentId)) return;

    bindNode(uuid, node);
    bindChildren(uuid);

    bindPendingChildren();
}

int NodeRegistry::countTagged() const noexcept
{
    //std::lock_guard<std::mutex> lock(m_lock);

    int count = m_taggedCount;
    if (count < 0) {
        count = 0;
        std::lock_guard<std::mutex> lock(m_snapshotLock);
        for (auto* node : m_allNodes) {
            if (node->isTagged()) count++;
        }
        m_taggedCount = count;
    }
    return count;
}

int NodeRegistry::countSelected() const noexcept
{
    //std::lock_guard<std::mutex> lock(m_lock);

    int count = m_selectedCount;
    if (count < 0) {
        count = 0;
        std::lock_guard<std::mutex> lock(m_snapshotLock);
        for (auto* node : m_allNodes) {
            if (node->isSelected()) count++;
        }
        m_selectedCount = count;
    }
    return count;
}

void NodeRegistry::changeParent(
    Node* node,
    const uuids::uuid& parentId) noexcept
{
    Node* parent = getNode(parentId);
    if (!parent) return;

    {
        Node* oldParent = node->getParent();
        if (oldParent == parent) return;

        if (oldParent) {
            oldParent->removeChild(node);
        }

        //auto& oldChildren = m_parentToChildren[oldParent->m_id];
        //const auto& it = std::remove_if(
        //    oldChildren.begin(),
        //    oldChildren.end(),
        //    [&node](auto& n) {
        //        return n->m_id == node->m_id;
        //    });
        //oldChildren.erase(it, oldChildren.end());
    }

    node->setParent(parent);
    parent->addChild(node);

    //auto& children = m_parentToChildren[parent->m_id];
    //children.push_back(node);
}

void NodeRegistry::bindNode(
    const uuids::uuid& uuid,
    Node* node)
{
    KI_INFO(fmt::format("BIND_NODE: {}", node->str()));

    const mesh::MeshType* type;
    {
        auto* t = m_registry->m_typeRegistry->modifyType(node->m_type->m_id);
        t->prepare({ m_assets, m_registry });

        type = t;
        node->m_type = type;
    }
    node->prepare({ m_assets, m_registry });

    {
        //KI_INFO_OUT(fmt::format(
        //    "REGISTER: {}-{}",
        //    program ? program->m_key : "<na>", programKey.str()));

        if (!uuid.is_nil()) m_uuidToNode[uuid] = node;

        {
            std::lock_guard<std::mutex> lock(m_snapshotLock);
            m_allNodes.push_back(node);
        }

        if (uuid == m_assets.rootUUID) {
            m_root = node;
        }
    }

    clearSelectedCount();

    // NOTE KI ensure related snapshots are visible in RT
    // => otherwise IOOBE will trigger
    m_registry->m_snapshotRegistry->copyToPending(node->m_snapshotIndex);

    {
        event::Event evt { event::Type::node_added };
        evt.body.node.target = node;
        m_registry->m_dispatcher->send(evt);
    }

    {
        event::Event evt { event::Type::node_added };
        evt.body.node.target = node;
        m_registry->m_dispatcherView->send(evt);
    }

    {
        event::Event evt { event::Type::type_prepare_view };
        evt.body.meshType.target = node->m_type->m_id;
        m_registry->m_dispatcherView->send(evt);
    }

    KI_INFO(fmt::format("ATTACH_NODE: node={}", node->str()));
}

void NodeRegistry::bindPendingChildren()
{
    if (m_pendingChildren.empty()) return;

    std::vector<uuids::uuid> boundIds;

    for (const auto& [parentId, children] : m_pendingChildren) {
        const auto& parentIt = m_uuidToNode.find(parentId);
        if (parentIt == m_uuidToNode.end()) continue;

        boundIds.push_back(parentId);

        auto& parent = parentIt->second;
        for (auto& [uuid, child] : children) {
            KI_INFO(fmt::format("BIND_CHILD: parent={}, child={}", parent->str(), child->str()));
            bindNode(uuid, child);

            child->setParent(parent);
            parent->addChild(child);
            //m_parentToChildren[parent->m_id].push_back(child);
        }
    }

    for (auto& parentId : boundIds) {
        m_pendingChildren.erase(parentId);
    }
}


bool NodeRegistry::bindParent(
    Node* child,
    const uuids::uuid& childUUID,
    const uuids::uuid& parentUUID,
    ki::node_id parentId)
{
    if (parentId) {
        auto* parent = m_idToNode.find(parentId)->second;
        child->setParent(parent);
        parent->addChild(child);
        //m_parentToChildren[parent->m_id].push_back(child);
        return true;
    }

    if (parentUUID.is_nil()) return true;

    const auto& parentIt = m_uuidToNode.find(parentUUID);
    if (parentIt == m_uuidToNode.end()) {
        KI_INFO(fmt::format("PENDING_CHILD: node={}", child->str()));

        m_pendingChildren[parentUUID].push_back({ childUUID, child });
        return false;
    }

    auto& parent = parentIt->second;
    KI_INFO(fmt::format("BIND_PARENT: parent={}, child={}", parent->str(), child->str()));

    child->setParent(parent);
    parent->addChild(child);
    //m_parentToChildren[parent->m_id].push_back(child);

    return true;
}

void NodeRegistry::bindChildren(
    const uuids::uuid& parentUUID)
{
    const auto& it = m_pendingChildren.find(parentUUID);
    if (it == m_pendingChildren.end()) return;

    Node* parent = m_uuidToNode.find(parentUUID)->second;

    for (auto& [uuid, child] : it->second) {
        KI_INFO(fmt::format("BIND_CHILD: parent={}, child={}", parent->str(), child->str()));
        bindNode(uuid, child);

        child->setParent(parent);
        parent->addChild(child);
        //m_parentToChildren[parent->m_id].push_back(child);
    }

    m_pendingChildren.erase(parentUUID);
}

const Material& NodeRegistry::getSelectionMaterial() const noexcept
{
    return *m_selectionMaterial;
}

void NodeRegistry::setSelectionMaterial(const Material& material)
{
    *m_selectionMaterial = material;
}

void NodeRegistry::setActiveNode(Node* node)
{
    if (!node) return;

    m_activeNode = node;
}

void NodeRegistry::setActiveCameraNode(Node* node)
{
    if (!node) return;
    if (!node->m_camera) return;

    m_activeCameraNode = node;
}

Node* NodeRegistry::getNextCameraNode(Node* srcNode, int offset) const noexcept
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

Node* NodeRegistry::findDefaultCameraNode() const
{
    const auto& it = std::find_if(
        m_cameraNodes.begin(),
        m_cameraNodes.end(),
        [](Node* node) { return node->m_camera->isDefault(); });
    return it != m_cameraNodes.end() ? *it : nullptr;
}

void NodeRegistry::bindSkybox(
    Node* node) noexcept
{
    const mesh::MeshType* type;
    {
        auto* t = m_registry->m_typeRegistry->modifyType(node->m_type->m_id);
        t->prepare({ m_assets, m_registry });

        type = t;
        node->m_type = type;
    }
    node->prepare({ m_assets, m_registry });

    m_skybox = node;

    {
        event::Event evt { event::Type::type_prepare_view };
        evt.body.meshType.target = node->m_type->m_id;
        m_registry->m_dispatcherView->send(evt);
    }
}
