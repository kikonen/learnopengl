#include "NodeRegistry.h"

#include "Scene.h"

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
        idToNode.clear();
        uuidToNode.clear();
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
    for (auto& byType : allNodes) {
        for (auto& e : byType.second) {
            KI_INFO_SB("NODE_REGISTRY: delete " << e.first->typeID);
            for (auto& n : e.second) {
                delete n;
            }
        }
    }
    allNodes.clear();
}

void NodeRegistry::addNode(Node* node)
{
    std::lock_guard<std::mutex> lock(load_lock);
    KI_INFO_SB("ADD_NODE: id=" << node->objectID << ", type=" << node->type->typeID);
    pendingNodes.push_back(node);
    uuidToNode[node->id] = node;

    waitCondition.notify_all();
}

Node* NodeRegistry::getNode(const uuids::uuid& id)
{
    std::lock_guard<std::mutex> lock(load_lock);
    const auto& entry = uuidToNode.find(id);
    if (entry == uuidToNode.end()) return nullptr;
    return entry->second;
}

Node* NodeRegistry::getNode(int objectID)
{
    if (idToNode.find(objectID) == idToNode.end()) return nullptr;
    return idToNode[objectID];
}


void NodeRegistry::selectNodeById(int objectID, bool append)
{
    if (!append) {
        for (auto& x : idToNode) {
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

    for (const auto& x : newNodes) {
        auto& t = x.first;
        t->prepare(assets);

        auto* shader = t->nodeShader;

        auto* map = &solidNodes;
        if (t->flags.alpha)
            map = &alphaNodes;
        if (t->flags.blend)
            map = &blendedNodes;

        auto& typeMap = (*map)[shader->objectID];
        auto& allMap = allNodes[shader->objectID];

        for (auto& node : x.second) {
            node->prepare(assets);

            allMap[t].push_back(node);
            typeMap[t].push_back(node);
            idToNode[node->objectID] = node;

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

            KI_INFO_SB("ATTACH_NODE: id=" << node->objectID << ", type=" << t->typeID);

            scene.bindComponents(node);
        }
    }
}

int NodeRegistry::countSelected() const
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
