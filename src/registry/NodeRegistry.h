#pragma once

#include <map>
#include <vector>
#include <tuple>

#include <fmt/format.h>

#include <ki/uuid.h>

#include "backend/DrawOptions.h"

#include "model/Group.h"
#include "model/Node.h"

#include "registry/MeshType.h"


class Registry;


//
// NOTE KI program key is REQUIRED for sorting "gull back face" draws
// next to each other to avoid redundant state changes
// => relies into fact that std::map is sorted by this
//
 struct ProgramKey {
    ProgramKey(
        int programID,
        int typePriority,
        const backend::DrawOptions& drawOptions) noexcept
        : programID(programID),
        typePriority(typePriority),
        renderBack(drawOptions.renderBack),
        wireframe(drawOptions.wireframe)
    {};

    const std::string str() const noexcept
    {
        return fmt::format(
            "<PROGRAM_KEY: id={}, pri={}, renderBack={}, wireframe={}>",
            programID, typePriority, renderBack, wireframe);
    }

    bool operator<(const ProgramKey & o) const noexcept {
        // NOTE KI renderBack & wireframe goes into separate render always due to GL state
        // => reduce state changes via sorting
        return std::tie(typePriority, programID, renderBack, wireframe) <
            std::tie(o.typePriority, o.programID, o.renderBack, o.wireframe);
    }

    const int typePriority;
    const int programID;
    const bool renderBack;
    const bool wireframe;
};

// https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
struct MeshTypeKey {
    MeshTypeKey(MeshType* type);

    bool operator<(const MeshTypeKey& o) const;

    const MeshType* type;
};

using GroupVector = std::vector<Group*>;

using NodeVector = std::vector<Node*>;
using MeshTypeMap = std::map<MeshTypeKey, NodeVector>;
using ProgramTypeMap = std::map<ProgramKey, MeshTypeMap>;


class NodeRegistry final
{
public:
    NodeRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~NodeRegistry();

    void prepare(
        Registry* registry);

    void addGroup(Group* group) noexcept;

    inline bool containsNode(const int objectID) const noexcept
    {
        const auto& it = objectIdToNode.find(objectID);
        return it != objectIdToNode.end();
    }

    // @return node null if not found
    Node* getNode(const int objectID) const noexcept;

    // @return node null if not found
    Node* getNode(const uuids::uuid& id) const noexcept;

    void selectNodeByObjectId(int objectID, bool append) const noexcept;

    int countTagged() const noexcept;
    int countSelected() const noexcept;

    void changeParent(
        Node* node,
        const uuids::uuid& parentId) noexcept;

    inline const NodeVector* getChildren(const Node& parent) const noexcept {
        const auto& it = m_parentToChildren.find(parent.m_objectID);
        return it != m_parentToChildren.end() ? &it->second : nullptr;
    }

    Node* getActiveCamera() const { return m_activeCamera; }
    void setActiveCamera(Node* node);

    Node* findDefaultCamera() const;

private:
    void attachNode(
        Node* node,
        const uuids::uuid& parentId) noexcept;

        void insertNode(NodeVector& list, Node* node);

    void bindPendingChildren();

    void bindNode(
        Node* node);

    bool bindParent(
        Node* child,
        const uuids::uuid& parentId);

    void bindChildren(
        Node* parent);

public:
    std::map<int, Node*> objectIdToNode;
    std::map<uuids::uuid, Node*> idToNode;

    ProgramTypeMap allNodes;
    ProgramTypeMap solidNodes;
    ProgramTypeMap alphaNodes;
    ProgramTypeMap blendedNodes;
    ProgramTypeMap invisibleNodes;
    ProgramTypeMap physicsNodes;

    Node* m_root{ nullptr };
    Node* m_dirLight{ nullptr };

    NodeVector m_cameras;

    NodeVector m_pointLights;
    NodeVector m_spotLights;

    GroupVector groups;

    Material m_selectionMaterial;

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    Registry* m_registry{ nullptr };

    std::map<uuids::uuid, NodeVector> m_pendingChildren;

    std::map<int, NodeVector> m_parentToChildren;

    Node* m_activeCamera{ nullptr };
};
