#include "NodeRegistry.h"

#include <fmt/format.h>

#include "ki/GL.h"

#include "asset/Program.h"

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"

#include "renderer/SkyboxRenderer.h"

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
    m_pendingNodes.reserve(1000);
    m_newNodes.reserve(1000);
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

    for (auto& group : groups) {
        delete group;
    }
    groups.clear();
}

void NodeRegistry::prepare(
    Registry* registry)
{
    m_registry = registry;

    m_selectionMaterial = Material::createMaterial(BasicMaterial::selection);
    registry->m_materialRegistry->add(m_selectionMaterial);
}

void NodeRegistry::addListener(NodeListener& listener)
{
    m_listeners.push_back(listener);
}

void NodeRegistry::addGroup(Group* group) noexcept
{
    if (!*m_alive) return;

    std::lock_guard<std::mutex> lock(m_load_lock);
    groups.push_back(group);
}

void NodeRegistry::addNode(
    MeshType* type,
    Node* node) noexcept
{
    if (!*m_alive) return;

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
            x.second->setSelectionMaterialIndex(-1);
        }
    }

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

void NodeRegistry::addViewPort(std::shared_ptr<Viewport> viewport) noexcept
{
    std::lock_guard<std::mutex> lock(m_load_lock);
    viewports.push_back(viewport);
}

void NodeRegistry::attachNodes()
{
    {
        std::lock_guard<std::mutex> lock(m_load_lock);

        if (m_pendingNodes.empty()) return;

        m_newNodes.insert(
            m_newNodes.end(),
            m_pendingNodes.begin(),
            m_pendingNodes.end());

        m_pendingNodes.clear();
    }

    for (auto& node : m_newNodes) {
        // NOTE KI ignore children without parent; until parent is found
        if (!bindParent(node)) continue;

        bindNode(node);
        bindChildren(node);
    }

    bindPendingChildren();

    if (m_skybox && !m_skyboxPrepared) {
        m_skyboxPrepared = true;
        m_skybox->prepare(
            m_assets,
            m_registry);
    }

    m_newNodes.clear();
}

int NodeRegistry::countTagged() const noexcept
{
    int count = 0;
    for (const auto& all : allNodes) {
        for (const auto& x : all.second) {
            for (auto& node : x.second) {
                if (node->isTagged()) count++;
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
                if (node->isSelected()) count++;
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

    {
        type->modifyMaterials([this](Material& m) {
            m_registry->m_materialRegistry->add(m);
        });
    }

    type->prepare(m_assets, m_registry);
    node->prepare(m_assets, m_registry);

    {
        auto* map = &solidNodes;

        if (type->m_flags.alpha)
            map = &alphaNodes;

        if (type->m_flags.blend)
            map = &blendedNodes;

        if (type->m_flags.noRender)
            map = &invisibleNodes;

        // NOTE KI more optimal to not switch between culling mode (=> group by it)
        const ProgramKey programKey(
            program ? program->m_objectID : NULL_PROGRAM_ID,
            -type->m_priority,
            type->m_drawOptions);

        //KI_INFO_OUT(fmt::format(
        //    "REGISTER: {}-{}",
        //    program ? program->m_key : "<na>", programKey.str()));

        const MeshTypeKey typeKey(type);

        auto& vAll = allNodes[programKey][typeKey];
        auto& vTyped = (*map)[programKey][typeKey];

        objectIdToNode[node->m_objectID] = node;
        if (!node->m_id.is_nil()) idToNode[node->m_id] = node;

        insertNode(vAll, node);
        insertNode(vTyped, node);

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

        if (type->m_flags.root) {
            m_root = node;
        }
    }

    notifyListeners(node, NodeOperation::ADDED);

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
