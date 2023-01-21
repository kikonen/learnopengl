#pragma once

#include <map>
#include <vector>
#include <mutex>
#include <tuple>

#include <ki/uuid.h>

#include "backend/DrawOptions.h"

#include "model/Group.h"
#include "model/Node.h"
#include "model/Viewport.h"

#include "registry/MeshType.h"

class Batch;
class MaterialRegistry;
class EntityRegistry;
class ModelRegistry;

class SkyboxRenderer;

//
// NOTE KI shader key is REQUIRED for sorting "gull back face" draws
// next to each other to avoid redundant state changes
// => relies into fact that std::map is sorted by this
//
 struct ShaderKey {
    ShaderKey(int shaderID, const backend::DrawOptions& drawOptions) noexcept
        : shaderID(shaderID),
        renderBack(drawOptions.renderBack),
        wireframe(drawOptions.wireframe)
    {};

    bool operator<(const ShaderKey & o) const noexcept {
        // NOTE KI renderBack & wireframe goes into separate render always due to GL state
        // => reduce state changes via sorting
        return std::tie(shaderID, renderBack, wireframe) <
            std::tie(o.shaderID, o.renderBack, o.wireframe);
    }

    const int shaderID;
    const bool renderBack;
    const bool wireframe;
};

// https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
struct MeshTypeKey {
    MeshTypeKey(MeshType* type);

    bool operator<(const MeshTypeKey& o) const;

    const MeshType* type;
};

enum class NodeOperation {
    ADDED
};

using GroupVector = std::vector<Group*>;

using NodeVector = std::vector<Node*>;
using MeshTypeMap = std::map<MeshTypeKey, NodeVector>;
using ShaderTypeMap = std::map<ShaderKey, MeshTypeMap>;

using ViewportVector = std::vector<std::shared_ptr<Viewport>>;

using NodeListener = std::function<void(Node*, NodeOperation)>;


class NodeRegistry final
{
public:
    NodeRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~NodeRegistry();

    void prepare(
        Batch* batch,
        MaterialRegistry * materialRegistry,
        EntityRegistry* entityRegistry,
        ModelRegistry* modelRegistry);

    void addListener(NodeListener& listener);

    void addGroup(Group* group) noexcept;

    void addNode(
        MeshType* type,
        Node* node) noexcept;

    // @return node null if not found
    Node* getNode(const int objectID) const noexcept;

    // @return node null if not found
    Node* getNode(const uuids::uuid& id) const noexcept;

    void selectNodeByObjectId(int objectID, bool append) const noexcept;

    void addViewPort(std::shared_ptr<Viewport> viewport) noexcept;

    void attachNodes();

    int countTagged() const noexcept;
    int countSelected() const noexcept;

    void changeParent(Node* node, uuids::uuid parentId) noexcept;

    Node* getParent(const Node& node) const noexcept;
    const NodeVector* getChildren(const Node& node) const noexcept;

private:
    void insertNode(NodeVector& list, Node* node);

    void bindPendingChildren();

    void bindNode(
        Node* node);

    bool bindParent(
        Node* child);

    void bindChildren(
        Node* parent);

    void notifyListeners(Node* node, NodeOperation operation);

public:
    std::map<int, Node*> objectIdToNode;
    std::map<uuids::uuid, Node*> idToNode;

    ShaderTypeMap allNodes;
    ShaderTypeMap solidNodes;
    ShaderTypeMap alphaNodes;
    ShaderTypeMap blendedNodes;
    ShaderTypeMap invisibleNodes;

    ViewportVector viewports;

    std::unique_ptr<SkyboxRenderer> m_skybox;

    Node* m_root = nullptr;
    NodeVector m_cameraNodes;
    Node* m_dirLight = nullptr;

    NodeVector m_pointLights;
    NodeVector m_spotLights;

    GroupVector groups;

    Material m_selectionMaterial;

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    Batch* m_batch{ nullptr };
    MaterialRegistry* m_materialRegistry{ nullptr };
    EntityRegistry* m_entityRegistry{ nullptr };
    ModelRegistry* m_modelRegistry{ nullptr };

    std::mutex m_load_lock;
    std::condition_variable m_waitCondition;

    std::map<uuids::uuid, NodeVector> m_pendingChildren;

    std::map<int, Node*> m_childToParent;
    std::map<int, NodeVector> m_parentToChildren;

    NodeVector m_pendingNodes;
    NodeVector m_newNodes;

    std::vector<NodeListener> m_listeners;
};
