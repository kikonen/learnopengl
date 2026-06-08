#pragma once

#include <vector>
#include <map>
#include <cstdint>

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
    using NodeVector = std::vector<pool::NodeHandle>;

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

        // maintain the per-kind drawable-index buckets for a node's drawable range
        void addDrawables(model::Node* node);
        void removeDrawables(model::Node* node);

    public:
        // FOUNDATIONAL (not yet driving the draw): per-kind drawable-index buckets
        // (global indices into InstanceRegistry::m_drawables) the future sweep will iterate.
        // A drawable lands in every kind it matches (m_kindBits is a mask). Maintained on
        // node add/remove; instanceRef is fixed for the node lifetime so these never go stale.
        std::vector<uint32_t> m_solidDrawables;
        std::vector<uint32_t> m_alphaDrawables;
        std::vector<uint32_t> m_blendedDrawables;

        // debug: assert every bucketed index is in-bounds, active, and correctly kinded
        void validateDrawables() const;

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
