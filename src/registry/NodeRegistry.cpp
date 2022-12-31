#include "NodeRegistry.h"

#include <fmt/format.h>

#include "ki/GL.h"

#include "MaterialRegistry.h"
#include "ModelRegistry.h"

namespace {
    const NodeVector EMPTY_NODE_LIST;

    const int NULL_SHADER_ID = 0;
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


NodeRegistry::NodeRegistry(const Assets& assets)
    : assets(assets)
{
}

NodeRegistry::~NodeRegistry()
{
    // NOTE KI forbid access into deleted nodes
    {
        std::lock_guard<std::mutex> lock(m_load_lock);
        m_pendingNodes.clear();
        objectIdToNode.clear();
        idToNode.clear();

        m_childToParent.clear();
        m_parentToChildren.clear();
    }

    {
        solidNodes.clear();
        blendedNodes.clear();
        invisibleNodes.clear();

        m_cameraNodes.clear();

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

    for (auto& group : groups) {
        delete group;
    }
    groups.clear();
}

void NodeRegistry::prepare(
    Batch* batch,
    MaterialRegistry* materialRegistry,
    ModelRegistry* modelRegistry)
{
    m_batch = batch;
    m_materialRegistry = materialRegistry;
    m_modelRegistry = modelRegistry;
}

void NodeRegistry::addListener(NodeListener& listener)
{
    m_listeners.push_back(listener);
}

void NodeRegistry::addGroup(Group* group) noexcept
{
    std::lock_guard<std::mutex> lock(m_load_lock);
    groups.push_back(group);
}

void NodeRegistry::addNode(
    MeshType* type,
    Node* node) noexcept
{
    std::lock_guard<std::mutex> lock(m_load_lock);
    KI_INFO(fmt::format("ADD_NODE: {}", node->str()));
    m_pendingNodes.push_back(node);

    m_waitCondition.notify_all();
}

Node* NodeRegistry::getNode(const uuids::uuid& id) const noexcept
{
    if (id.is_nil()) return nullptr;

    //std::lock_guard<std::mutex> lock(load_lock);
    const auto& it = idToNode.find(id);
    return it != idToNode.end() ? it->second : nullptr;
}

Node* NodeRegistry::getNode(const int objectID) const noexcept
{
    const auto& it = objectIdToNode.find(objectID);
    return it != objectIdToNode.end() ? it->second : nullptr;
}

void NodeRegistry::selectNodeByObjectId(int objectID, bool append) const noexcept
{
    if (!append) {
        for (auto& x : objectIdToNode) {
            x.second->m_selected = false;
        }
    }

    Node* node = getNode(objectID);
    if (!node) return;

    if (append && node->m_selected) {
        KI_INFO_SB("DESELECT: objectID: " << objectID);
        node->m_selected = false;
    }
    else {
        KI_INFO_SB("SELECT: objectID: " << objectID);
        node->m_selected = true;
    }
}

void NodeRegistry::addViewPort(std::shared_ptr<Viewport> viewport) noexcept
{
    std::lock_guard<std::mutex> lock(m_load_lock);
    viewports.push_back(viewport);
}

void NodeRegistry::attachNodes()
{
    MeshTypeMap newNodes;
    {
        std::lock_guard<std::mutex> lock(m_load_lock);
        if (m_pendingNodes.empty()) return;

        for (const auto& node : m_pendingNodes) {
            newNodes[node->m_type].push_back(node);
        }
        m_pendingNodes.clear();
    }

    for (const auto& [type, nodes] : newNodes) {
        for (auto& node : nodes) {
            // NOTE KI ignore children without parent; until parent is found
            if (!bindParent(node)) continue;

            bindNode(node);
        }

        for (auto& node : nodes) {
            bindChildren(node);
        }
    }

    bindPendingChildren();
}

int NodeRegistry::countHighlighted() const noexcept
{
    int count = 0;
    for (const auto& all : allNodes) {
        for (const auto& x : all.second) {
            for (auto& node : x.second) {
                if (node->m_highlighted) count++;
            }
        }
    }
    return count;
}

int NodeRegistry::countSelected() const noexcept
{
    int count = 0;
    for (const auto& all : allNodes) {
        for (const auto& x : all.second) {
            for (auto& node : x.second) {
                if (node->m_selected) count++;
            }
        }
    }
    return count;
}

void NodeRegistry::changeParent(
    Node* node,
    uuids::uuid parentId) noexcept
{
    Node* parent = getNode(parentId);
    if (!parent) return;

    {
        Node* oldParent = getNode(node->m_parentId);
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

    auto& children = m_parentToChildren[parent->m_objectID];

    children.push_back(node);
    m_childToParent[node->m_objectID] = parent;
    node->m_parentId = parent->m_id;
}

Node* NodeRegistry::getParent(const Node& child) const noexcept
{
    const auto& it = m_childToParent.find(child.m_objectID);
    return it != m_childToParent.end() ? it->second : nullptr;
}

const NodeVector* NodeRegistry::getChildren(const Node& parent) const noexcept
{
    const auto& it = m_parentToChildren.find(parent.m_objectID);
    return it != m_parentToChildren.end() ? &it->second : nullptr;
}

void NodeRegistry::bindNode(
    Node* node)
{
    KI_INFO(fmt::format("BIND_NODE: {}", node->str()));

    const auto& type = node->m_type;
    auto* shader = type->m_nodeShader;

    if (type->m_entityType != EntityType::origo) {
        assert(shader);
        if (!shader) return;
    }

    {
        type->modifyMaterials([this](Material& m) {
            m_materialRegistry->add(m);
        });
    }

        type->prepare(assets, *m_batch, *this, *m_materialRegistry, *m_modelRegistry);
    node->prepare(assets);

    {
        auto* map = &solidNodes;

        if (type->m_flags.alpha)
            map = &alphaNodes;

        if (type->m_flags.blend)
            map = &blendedNodes;

        if (type->m_flags.noRender)
            map = &invisibleNodes;

        // NOTE KI more optimal to not switch between culling mode (=> group by it)
        const ShaderKey shaderKey(
            shader ? shader->m_objectID : NULL_SHADER_ID,
            type->m_drawOptions);

        const MeshTypeKey typeKey(type);

        auto& vAll = allNodes[shaderKey][typeKey];
        auto& vTyped = (*map)[shaderKey][typeKey];

        objectIdToNode[node->m_objectID] = node;
        if (!node->m_id.is_nil()) idToNode[node->m_id] = node;

        insertNode(vAll, node);
        insertNode(vTyped, node);

        if (node->m_camera) {
            m_cameraNodes.push_back(node);
        }

        if (node->m_light) {
            Light* light = node->m_light.get();

            if (light->directional) {
                m_dirLight = node;
            }
            else if (light->point) {
                m_pointLights.push_back(node);
            }
            else if (light->spot) {
                m_spotLights.push_back(node);
            }
        }

        if (type->m_flags.root) {
            m_root = node;
        }
    }

    notifyListeners(node, NodeOperation::ADDED);

    KI_INFO_SB("ATTACH_NODE: id=" << node->str());
}

void NodeRegistry::insertNode(NodeVector& list, Node* node)
{
    list.push_back(node);
}

void NodeRegistry::bindPendingChildren()
{
    if (m_pendingChildren.empty()) return;

    std::vector<uuids::uuid> boundIds;

    for (const auto& [parentId, children] : m_pendingChildren) {
        const auto& parentIt = idToNode.find(parentId);
        if (parentIt == idToNode.end()) continue;

        boundIds.push_back(parentId);

        auto& parent = parentIt->second;
        for (auto& child : children) {
            KI_INFO(fmt::format("BIND_CHILD: parent={}, child={}", parent->str(), child->str()));
            bindNode(child);

            m_childToParent[child->m_objectID] = parent;
            m_parentToChildren[parent->m_objectID].push_back(child);
        }
    }

    for (auto& parentId : boundIds) {
        m_pendingChildren.erase(parentId);
    }
}


bool NodeRegistry::bindParent(
    Node* child)
{
    if (child->m_parentId.is_nil()) return true;

    const auto& parentIt = idToNode.find(child->m_parentId);
    if (parentIt == idToNode.end()) {
        KI_INFO(fmt::format("PENDING_CHILD: node={}", child->str()));

        m_pendingChildren[child->m_parentId].push_back(child);
        return false;
    }

    auto& parent = parentIt->second;
    KI_INFO(fmt::format("BIND_PARENT: parent={}, child={}", parent->str(), child->str()));

    m_childToParent[child->m_objectID] = parent;
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

        m_childToParent[child->m_objectID] = parent;
        m_parentToChildren[parent->m_objectID].push_back(child);
    }

    m_pendingChildren.erase(parent->m_id);
}

void NodeRegistry::notifyListeners(Node* node, NodeOperation operation)
{
    for (auto& listener : m_listeners) {
        listener(node, operation);
    }
}
