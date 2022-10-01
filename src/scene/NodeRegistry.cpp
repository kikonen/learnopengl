#include "NodeRegistry.h"

#include "Scene.h"


namespace {
    const NodeVector EMPTY_NODE_LIST;

    const int NULL_SHADER_ID = 0;
}

NodeRegistry::NodeRegistry(Scene& scene)
    : scene(scene),
    assets(scene.assets)
{
}

NodeRegistry::~NodeRegistry()
{
    // NOTE KI forbid access into deleted nodes
    {
        std::lock_guard<std::mutex> lock(load_lock);
        pendingNodes.clear();
        objectIdToNode.clear();
        idToNode.clear();

        childToParent.clear();
        parentToChildren.clear();
    }

    {
        solidNodes.clear();
        blendedNodes.clear();

        cameraNode = nullptr;

        dirLight = nullptr;
        pointLights.clear();
        spotLights.clear();
    }

    KI_INFO_SB("NODE_REGISTRY: delete");
    for (auto& all : allNodes) {
        for (auto& [type, nodes] : all.second) {
            KI_INFO_SB("NODE_REGISTRY: delete " << type->typeID);
            for (auto& node : nodes) {
                delete node;
            }
        }
    }
    allNodes.clear();

    for (auto& [parentId, nodes] : pendingChildren) {
        for (auto& node : nodes) {
            delete node;
        }
    }
    pendingChildren.clear();

    for (auto& group : groups) {
        delete group;
    }
    groups.clear();
}

void NodeRegistry::addGroup(Group* group)
{
    std::lock_guard<std::mutex> lock(load_lock);
    groups.push_back(group);
}

void NodeRegistry::addNode(Node* node)
{
    std::lock_guard<std::mutex> lock(load_lock);
    KI_INFO_SB("ADD_NODE: id=" << node->objectID << ", type=" << node->type->str());
    pendingNodes.push_back(node);

    waitCondition.notify_all();
}

Node* const NodeRegistry::getNode(const uuids::uuid& id)
{
    if (id.is_nil()) return nullptr;

    //std::lock_guard<std::mutex> lock(load_lock);
    const auto& it = idToNode.find(id);
    return it != idToNode.end() ? it->second : nullptr;
}

Node* const NodeRegistry::getNode(const int objectID)
{
    const auto& it = objectIdToNode.find(objectID);
    return it != objectIdToNode.end() ? it->second : nullptr;
}


void const NodeRegistry::selectNodeByObjectId(int objectID, bool append)
{
    if (!append) {
        for (auto& x : objectIdToNode) {
            x.second->selected = false;
        }
    }

    Node* node = getNode(objectID);
    if (node) {
        if (append && node->selected) {
            KI_INFO_SB("DESELECT: objectID: " << objectID)
                node->selected = false;
        }
        else {
            KI_INFO_SB("SELECT: objectID: " << objectID)
                node->selected = true;
        }
    }
}

void NodeRegistry::addViewPort(std::shared_ptr<Viewport> viewport)
{
    std::lock_guard<std::mutex> lock(load_lock);
    viewports.push_back(viewport);
}

void NodeRegistry::attachNodes()
{
    std::lock_guard<std::mutex> lock(load_lock);
    if (pendingNodes.empty()) return;

    NodeTypeMap newNodes;

    {
        for (const auto& n : pendingNodes) {
            newNodes[n->type.get()].push_back(n);
        }
        pendingNodes.clear();
    }

    for (const auto& [type, nodes] : newNodes) {
        for (auto& node : nodes) {
            //if (node->groupId == KI_UUID("fb7ba424-f219-4c12-a00f-703b9ecebbbf")) {
            //    KI_BREAK();
            //}

            // NOTE KI ignore children without parent; until parent is found
            if (!bindParent(node)) continue;

            bindNode(node);
        }

        for (auto& node : nodes) {
            bindChildren(node);
        }
    }
}

int const NodeRegistry::countSelected() const
{
    int count = 0;
    for (const auto& all : allNodes) {
        for (const auto& x : all.second) {
            for (auto& node : x.second) {
                if (node->selected) count++;
            }
        }
    }
    return count;
}

Node* const NodeRegistry::getParent(const Node& child)
{
    const auto& it = childToParent.find(child.objectID);
    return it != childToParent.end() ? it->second : nullptr;
}

NodeVector* const NodeRegistry::getChildren(const Node& parent)
{
    const auto& it = parentToChildren.find(parent.objectID);
    return it != parentToChildren.end() ? &it->second : nullptr;
}

void NodeRegistry::bindNode(Node* node)
{
    const auto& type = node->type.get();
    auto* shader = type->nodeShader;

    assert(shader);
    if (!shader) return;

    //if (node->id == KI_UUID("7c90bc35-1a05-4755-b52a-1f8eea0bacfa")) KI_BREAK();

    type->prepare(assets);

    auto* map = &solidNodes;

    if (type->flags.alpha)
        map = &alphaNodes;

    if (type->flags.blend)
        map = &blendedNodes;

    // NOTE KI more optimal to not switch between culling mode (=> group by it)
    const ShaderKey key(shader ? shader->objectID : NULL_SHADER_ID, type->flags.renderBack);

    auto& vAll = allNodes[key][type];
    auto& vTyped = (*map)[key][type];

    node->prepare(assets);

    objectIdToNode[node->objectID] = node;
    if (!node->id.is_nil()) idToNode[node->id] = node;

    vAll.push_back(node);
    vTyped.push_back(node);

    if (node->camera) {
        cameraNode = node;
    }

    if (node->light) {
        Light* light = node->light.get();

        if (light->directional) {
            dirLight = node;
        }
        else if (light->point) {
            pointLights.push_back(node);
        }
        else if (light->spot) {
            spotLights.push_back(node);
        }
    }

    scene.bindComponents(node);

    KI_INFO_SB("ATTACH_NODE: id=" << node->objectID << ", uuid=" << node->id << ", type=" << type->str());
}

bool NodeRegistry::bindParent(Node* child)
{
    if (child->parentId.is_nil()) return true;

    auto parent = getNode(child->parentId);
    if (parent) {
        KI_INFO_SB("BIND_PARENT: "
            << parent->id << "(" << parent->objectID << ") => "
            << child->id << "(" << child->objectID << "), type="
            << child->type->str());
        childToParent[child->objectID] = parent;
        return true;
    }

    pendingChildren[child->parentId].push_back(child);
    return false;
}

void NodeRegistry::bindChildren(Node* parent)
{
    const auto& it = pendingChildren.find(parent->id);
    if (it == pendingChildren.end()) return;

    for (auto& child : it->second) {
        KI_INFO_SB("BIND_CHILD: " << parent->id << " => " << child->id << ", type=" << child->type->str());
        bindNode(child);
        parentToChildren[parent->objectID].push_back(child);
    }

    pendingChildren.erase(parent->id);
}
