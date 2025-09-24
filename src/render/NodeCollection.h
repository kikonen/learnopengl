#pragma once

#include <vector>
#include <map>

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

namespace model
{
    class Node;
}

struct UpdateContext;

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

        void updateRT(const UpdateContext& ctx);

        inline model::Node* getActiveCameraNode() const noexcept
        {
            return m_activeCameraNode.toNode();
        }

        void setActiveCameraNode(pool::NodeHandle node);

        pool::NodeHandle getNextCameraNode(
            pool::NodeHandle srcNode,
            int offset) const noexcept;

        pool::NodeHandle findDefaultCameraNode() const;

        const pool::NodeHandle& getDirLightNode() const noexcept
        {
            return m_dirLightNodes.empty() ? pool::NodeHandle::NULL_HANDLE : m_dirLightNodes[0];
        }

        const std::vector<pool::NodeHandle>& getPointLightNodes() const noexcept
        {
            return m_pointLightNodes;
        }

        const std::vector<pool::NodeHandle>& getSpotLightNodes() const noexcept
        {
            return m_spotLightNodes;
        }

        void handleNodeAdded(model::Node* node);
        void handleNodeRemoved(model::Node* node);

    private:
        void insertNode(
            NodeVector* nodes,
            pool::NodeHandle nodeHandle);

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

        //std::vector<NodeComponent<CameraComponent>> m_cameraComponents;

        pool::NodeHandle m_activeCameraNode{};
        std::vector<pool::NodeHandle> m_cameraNodes;

        std::vector<pool::NodeHandle> m_dirLightNodes;
        std::vector<pool::NodeHandle> m_pointLightNodes;
        std::vector<pool::NodeHandle> m_spotLightNodes;

    };
}
