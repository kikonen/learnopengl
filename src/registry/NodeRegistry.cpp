#include "NodeRegistry.h"

#include <algorithm>
#include <glm/gtx/quaternion.hpp>

#include <fmt/format.h>

#include "ki/limits.h"

#include "kigl/kigl.h"

#include "asset/Program.h"

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"

#include "event/Dispatcher.h"

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

namespace {
    const NodeVector EMPTY_NODE_LIST;

    const ki::program_id NULL_PROGRAM_ID = 0;
}


// https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
MeshTypeKey::MeshTypeKey(const MeshType* type)
    : type(type)
{}

bool MeshTypeKey::operator<(const MeshTypeKey& o) const {
    const auto& a = type;
    const auto& b = o.type;
    if (a->m_drawOptions < b->m_drawOptions) return true;
    else if (b->m_drawOptions < a->m_drawOptions) return false;
    return a->m_id < b->m_id;
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
    // NOTE KI forbid access into deleted nodes
    {
        m_idToNode.clear();
        m_uuidToNode.clear();

        //m_parentToChildren.clear();
    }

    {
        solidNodes.clear();
        blendedNodes.clear();
        invisibleNodes.clear();

        m_activeNode = nullptr;
        m_activeCamera = nullptr;
        m_cameras.clear();

        m_dirLight = nullptr;
        m_pointLights.clear();
        m_spotLights.clear();

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

    m_selectionMaterial = Material::createMaterial(BasicMaterial::selection);
    registry->m_materialRegistry->registerMaterial(m_selectionMaterial);

    attachListeners();
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
            if (!node) node = findDefaultCamera();
            setActiveCamera(node);
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

    dispatcher->addListener(
        event::Type::scene_loaded,
        [this](const event::Event& e) {
            this->m_registry->m_physicsEngine->setEnabled(true);

            // NOTE KI trigger UI sidew update *after* all worker side processing done
            {
                event::Event evt { event::Type::scene_loaded };
                m_registry->m_dispatcherView->send(evt);
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
            type->prepareView(m_assets, m_registry);
        });
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
        node->setSelectionMaterialIndex(m_selectionMaterial.m_registeredIndex);
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
    int count = m_taggedCount;
    if (count < 0) {
        count = 0;
        for (auto* node : m_allNodes) {
            if (node->isTagged()) count++;
        }
        m_taggedCount = count;
    }
    return count;
}

int NodeRegistry::countSelected() const noexcept
{
    int count = m_selectedCount;
    if (count < 0) {
        count = 0;
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

    const MeshType* type;
    {
        auto* t = m_registry->m_typeRegistry->modifyType(node->m_type->m_id);
        t->prepare(m_assets, m_registry);

        type = t;
        node->m_type = type;
    }
    node->prepare(m_assets, m_registry);

    //const auto& type = node->m_type;
    auto* program = type->m_program;

    if (type->m_entityType != EntityType::origo) {
        assert(program);
        if (!program) return;
    }

    {
        // NOTE KI more optimal to not switch between culling mode (=> group by it)
        const ProgramKey programKey(
            program ? program->m_id : NULL_PROGRAM_ID,
            -type->m_priority,
            type->m_drawOptions);

        //KI_INFO_OUT(fmt::format(
        //    "REGISTER: {}-{}",
        //    program ? program->m_key : "<na>", programKey.str()));

        const MeshTypeKey typeKey(type);

        if (!uuid.is_nil()) m_uuidToNode[uuid] = node;

        {
            m_allNodes.push_back(node);
        }

        {
            auto* map = &solidNodes;

            if (type->m_flags.alpha)
                map = &alphaNodes;

            if (type->m_flags.blend)
                map = &blendedNodes;

            if (type->m_entityType == EntityType::sprite)
                map = &spriteNodes;

            if (type->m_flags.invisible)
                map = &invisibleNodes;

            auto& vTyped = (*map)[programKey][typeKey];
            insertNode(vTyped, node);
        }

        if (type->m_flags.enforceBounds) {
            m_physicsNodes.push_back(node);
        }

        if (node->m_camera) {
            m_cameras.push_back(node);
            if (!m_activeCamera && node->m_camera->isDefault()) {
                m_activeCamera = node;
            }
        }

        if (node->m_light) {
            Light* light = node->m_light.get();

            if (light->m_directional) {
                m_dirLight = node;
            }
            else if (light->m_point) {
                m_pointLights.push_back(node);
            }
            else if (light->m_spot) {
                m_spotLights.push_back(node);
            }
        }

        if (uuid == m_assets.rootUUID) {
            m_root = node;
        }
    }

    clearSelectedCount();

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

void NodeRegistry::insertNode(NodeVector& list, Node* node)
{
    list.reserve(100);
    list.push_back(node);
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

void NodeRegistry::setActiveNode(Node* node)
{
    if (!node) return;

    m_activeNode = node;
}

void NodeRegistry::setActiveCamera(Node* node)
{
    if (!node) return;
    if (!node->m_camera) return;

    m_activeCamera = node;
}

Node* NodeRegistry::getNextCamera(Node* srcNode, int offset) const noexcept
{
    int index = 0;
    int size = static_cast<int>(m_cameras.size());
    for (int i = 0; i < size; i++) {
        if (m_cameras[i] == srcNode) {
            index = std::max(0, (i + offset) % size);
            break;
        }
    }
    return m_cameras[index];
}

Node* NodeRegistry::findDefaultCamera() const
{
    const auto& it = std::find_if(
        m_cameras.begin(),
        m_cameras.end(),
        [](Node* node) { return node->m_camera->isDefault(); });
    return it != m_cameras.end() ? *it : nullptr;
}

void NodeRegistry::bindSkybox(
    Node* node) noexcept
{
    const MeshType* type;
    {
        auto* t = m_registry->m_typeRegistry->modifyType(node->m_type->m_id);
        t->prepare(m_assets, m_registry);

        type = t;
        node->m_type = type;
    }
    node->prepare(m_assets, m_registry);

    m_skybox = node;

    {
        event::Event evt { event::Type::type_prepare_view };
        evt.body.meshType.target = node->m_type->m_id;
        m_registry->m_dispatcherView->send(evt);
    }
}
