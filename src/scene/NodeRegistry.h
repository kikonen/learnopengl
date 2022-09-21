#pragma once

#include <map>
#include <vector>
#include <mutex>

#include <stduuid/uuid.h>

#include "NodeType.h"

#include "model/Node.h"
#include "model/Viewport.h"

class Sccene;

using NodeVector = std::vector<Node*>;
using NodeTypeMap = std::map<NodeType*, NodeVector>;
using ShaderTypeMap = std::map<int, NodeTypeMap>;

using ViewportVector = std::vector<std::shared_ptr<Viewport>>;

class NodeRegistry final
{
public:
    NodeRegistry(Scene& scene);

    ~NodeRegistry();

    void addNode(Node* node);

    // @return node null if not found
    Node* getNode(int objectID);

    // @return node null if not found
    Node* getNode(const uuids::uuid& id);

    void selectNodeById(int objectID, bool append);

    void addViewPort(std::shared_ptr<Viewport> viewport);

    void attachNodes();

private:

public:
    const Assets& assets;
    Scene& scene;

    std::map<int, Node*> idToNode;
    std::map<uuids::uuid, Node*> uuidToNode;

    NodeTypeMap nodes;

    ShaderTypeMap solidNodes;
    ShaderTypeMap blendedNodes;

    ViewportVector viewports;

    Node* cameraNode = nullptr;
    Node* dirLight = nullptr;

    NodeVector pointLights;
    NodeVector spotLights;

private:
    std::mutex load_lock;
    std::condition_variable waitCondition;

    NodeVector pendingNodes;

};
