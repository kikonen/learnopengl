#pragma once

#include <vector>
#include <map>

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

class Node;

namespace editor {
    class EditorFrame;
}

namespace render {
    class NodeDraw;

    // https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
    struct MeshTypeKey {
        // https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
        MeshTypeKey(pool::TypeHandle typeHandle)
            : m_typeHandle(typeHandle)
        {
        }

        bool operator<(const MeshTypeKey& o) const;

        const pool::TypeHandle m_typeHandle;
    };

    using NodeVector = std::vector<pool::NodeHandle>;
    using MeshTypeMap = std::map<MeshTypeKey, NodeVector>;


    class NodeCollection {
        friend class NodeDraw;

    public:
        NodeCollection();
        ~NodeCollection();

        void handleNodeAdded(Node* node);

    private:
        void insertNode(
            MeshTypeMap* map,
            Node* node);

    public:
        // NodeDraw
        MeshTypeMap m_solidNodes;
        // NodeDraw
        MeshTypeMap m_alphaNodes;
        // NodeDraw
        MeshTypeMap m_blendedNodes;
        // OBSOLETTE
        MeshTypeMap m_invisibleNodes;
    };
}
