#include "NodeRegistry.h"

#include <fmt/format.h>

#include "ki/GL.h"

#include "asset/Program.h"

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"

#include "event/Dispatcher.h"

#include "Registry.h"
#include "MaterialRegistry.h"
#include "EntityRegistry.h"
#include "ModelRegistry.h"

namespace {
    const NodeVector EMPTY_NODE_LIST;

    const int NULL_PROGRAM_ID = 0;
}


// https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
MeshTypeKey::MeshTypeKey(MeshType* type)
    : type(type)
{}

bool MeshTypeKey::operator<(const MeshTypeKey& o) const {
    const auto& a = type;
    const auto& b = o.type;
    if (a->m_drawOptions < b->m_drawOptions) return true;
    else if (b->m_drawOptions < a->m_drawOptions) return false;
    return a->typeID < b->typeID;
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
        m_objectIdToNode.clear();
        m_idToNode.clear();

        m_parentToChildren.clear();
    }

    {
        solidNodes.clear();
        blendedNodes.clear();
        invisibleNodes.clear();

        m_activeCamera = nullptr;
        m_cameras.clear();

        m_dirLight = nullptr;
        m_pointLights.clear();
        m_spotLights.clear();

        m_root = nullptr;
    }

    KI_INFO("NODE_REGISTRY: delete");
    for (auto& all : allNodes) {
        for (auto& [key, nodes] : all.second) {
            KI_INFO(fmt::format("NODE_REGISTRY: delete {}", key.type->str()));
            for (auto& node : nodes) {
                delete node;
            }
        }
    }
    allNodes.clear();

    for (auto& [parentId, nodes] : m_pendingChildren) {
        for (auto& node : nodes) {
            delete node;
        }
    }
    m_pendingChildren.clear();
}

void NodeRegistry::prepare(
    Registry* registry)
{
    m_registry = registry;

    m_selectionMaterial = Material::createMaterial(BasicMaterial::selection);
    registry->m_materialRegistry->add(m_selectionMaterial);

    m_registry->m_dispatcher->addListener(
        event::Type::node_add,
        [this](const event::Event& e) {
            attachNode(
                e.body.node.target,
                e.body.node.parentId);
        });

    m_registry->m_dispatcher->addListener(
        event::Type::node_change_parent,
        [this](const event::Event& e) {
            changeParent(e.body.node.target, e.body.node.parentId);
        });

    m_registry->m_dispatcher->addListener(
        event::Type::node_select,
        [this](const event::Event& e) {
            auto& node = e.body.node.target;
            node->setSelectionMaterialIndex(getSelectionMaterial().m_registeredIndex);
        });
}

void NodeRegistry::selectNodeByObjectId(int objectID, bool append) const noexcept
{
    if (!append) {
        for (auto& x : m_objectIdToNode) {
            x.second->setSelectionMaterialIndex(-1);
        }
    }

    clearSelectedCount();

    Node* node = getNode(objectID);
    if (!node) return;

    if (append && node->isSelected()) {
        KI_INFO(fmt::format("DESELECT: objectID={}", objectID));
        node->setSelectionMaterialIndex(-1);
    }
    else {
        KI_INFO(fmt::format("SELECT: objectID={}", objectID));
        node->setSelectionMaterialIndex(m_selectionMaterial.m_registeredIndex);
    }
}

void NodeRegistry::attachNode(
    Node* node,
    const uuids::uuid& parentId) noexcept
{
    m_objectIdToNode.insert(std::make_pair(node->m_objectID, node));

    // NOTE KI ignore children without parent; until parent is found
    if (!bindParent(node, parentId)) return;

    bindNode(node);
    bindChildren(node);

    bindPendingChildren();
}

int NodeRegistry::countTagged() const noexcept
{
    int count = m_taggedCount;
    if (count < 0) {
        count = 0;
        for (const auto& all : allNodes) {
            for (const auto& x : all.second) {
                for (auto& node : x.second) {
                    if (node->isTagged()) count++;
                }
            }
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
        for (const auto& all : allNodes) {
            for (const auto& x : all.second) {
                for (auto& node : x.second) {
                    if (node->isSelected()) count++;
                }
            }
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

        auto& oldChildren = m_parentToChildren[oldParent->m_objectID];
        const auto& it = std::remove_if(
            oldChildren.begin(),
            oldChildren.end(),
            [&node](auto& n) {
                return n->m_objectID == node->m_objectID;
            });
        oldChildren.erase(it, oldChildren.end());
    }

    node->setParent(parent);

    auto& children = m_parentToChildren[parent->m_objectID];
    children.push_back(node);
}

void NodeRegistry::bindNode(
    Node* node)
{
    KI_INFO(fmt::format("BIND_NODE: {}", node->str()));

    const auto& type = node->m_type;
    auto* program = type->m_program;

    if (type->m_entityType != EntityType::origo) {
        assert(program);
        if (!program) return;
    }

    type->prepare(m_assets, m_registry);
    node->prepare(m_assets, m_registry);

    {
        // NOTE KI more optimal to not switch between culling mode (=> group by it)
        const ProgramKey programKey(
            program ? program->m_objectID : NULL_PROGRAM_ID,
            -type->m_priority,
            type->m_drawOptions);

        //KI_INFO_OUT(fmt::format(
        //    "REGISTER: {}-{}",
        //    program ? program->m_key : "<na>", programKey.str()));

        const MeshTypeKey typeKey(type);

        if (!node->m_id.is_nil()) m_idToNode[node->m_id] = node;

        {
            auto& vAll = allNodes[programKey][typeKey];
            insertNode(vAll, node);
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

        if (type->m_flags.enforceBounds || type->m_flags.physics) {
            auto& vPhysics = physicsNodes[programKey][typeKey];
            insertNode(vPhysics, node);
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

        if (node->m_id == m_assets.rootUUID) {
            m_root = node;
        }
    }

    clearSelectedCount();

    event::Event evt { event::Type::node_added };
    evt.body.node.target = node;
    m_registry->m_dispatcher->send(evt);

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
        const auto& parentIt = m_idToNode.find(parentId);
        if (parentIt == m_idToNode.end()) continue;

        boundIds.push_back(parentId);

        auto& parent = parentIt->second;
        for (auto& child : children) {
            KI_INFO(fmt::format("BIND_CHILD: parent={}, child={}", parent->str(), child->str()));
            bindNode(child);

            child->setParent(parent);
            m_parentToChildren[parent->m_objectID].push_back(child);
        }
    }

    for (auto& parentId : boundIds) {
        m_pendingChildren.erase(parentId);
    }
}


bool NodeRegistry::bindParent(
    Node* child,
    const uuids::uuid& parentId)
{
    if (parentId.is_nil()) return true;

    const auto& parentIt = m_idToNode.find(parentId);
    if (parentIt == m_idToNode.end()) {
        KI_INFO(fmt::format("PENDING_CHILD: node={}", child->str()));

        m_pendingChildren[parentId].push_back(child);
        return false;
    }

    auto& parent = parentIt->second;
    KI_INFO(fmt::format("BIND_PARENT: parent={}, child={}", parent->str(), child->str()));

    child->setParent(parent);
    m_parentToChildren[parent->m_objectID].push_back(child);

    return true;
}

void NodeRegistry::bindChildren(
    Node* parent)
{
    const auto& it = m_pendingChildren.find(parent->m_id);
    if (it == m_pendingChildren.end()) return;

    for (auto& child : it->second) {
        KI_INFO(fmt::format("BIND_CHILD: parent={}, child={}", parent->str(), child->str()));
        bindNode(child);

        child->setParent(parent);
        m_parentToChildren[parent->m_objectID].push_back(child);
    }

    m_pendingChildren.erase(parent->m_id);
}

void NodeRegistry::setActiveCamera(Node* node)
{
    if (!node) return;
    if (!node->m_camera) return;

    m_activeCamera = node;
}

Node* NodeRegistry::findDefaultCamera() const
{
    const auto& it = std::find_if(
        m_cameras.begin(),
        m_cameras.end(),
        [](Node* node) { return node->m_camera->isDefault(); });
    return it != m_cameras.end() ? *it : nullptr;
}
