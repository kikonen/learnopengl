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
    //// https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
    //struct NodeTypeKey {
    //    // https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
    //    NodeTypeKey(pool::TypeHandle typeHandle)
    //        : m_typeHandle(typeHandle)
    //    {
    //    }

    //    bool operator<(const NodeTypeKey& o) const;

    //    const pool::TypeHandle m_typeHandle;
    //};

    using NodeVector = std::vector<pool::NodeHandle>;
    //using NodeTypeMap = std::map<NodeTypeKey, NodeVector>;


    // Collection of nodes in *single* scene
    // i.e. vs NodeRegistry which holds data over all scenes
    class NodeCollection {
        friend class NodeDraw;
        friend class WaterMapRenderer;
        friend class MirrorMapRenderer;
        friend class CubeMapMapRenderer;

    public:
        NodeCollection();
        ~NodeCollection();

        void clear();

        void handleNodeAdded(Node* node);

    private:
        void insertNode(
            NodeVector* nodes,
            Node* node);

    public:
        // NodeDraw
        NodeVector m_solidNodes;
        // NodeDraw
        NodeVector m_alphaNodes;
        // NodeDraw
        NodeVector m_blendedNodes;
        //// OBSOLETTE
        NodeVector m_invisibleNodes;

        std::vector<pool::NodeHandle> m_waterNodes;
        std::vector<pool::NodeHandle> m_mirrorNodes;
        std::vector<pool::NodeHandle> m_cubeMapNodes;
    };
}
