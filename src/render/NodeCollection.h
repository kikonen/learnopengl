#pragma once

#include <vector>
#include <map>
#include <array>
#include <cstdint>

#include "asset/LayerInfo.h"

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
        // maintain the per-kind drawable-index buckets for a node's drawable range
        void addDrawables(model::Node* node);
        void removeDrawables(model::Node* node);

    public:
        // Per-(layer, kind) drawable-index buckets (global indices into
        // InstanceRegistry::m_drawables) the sweep iterates. Keyed by node layer so an
        // unbalanced layer (e.g. ui) doesn't scan the main set. A drawable lands in every
        // kind it matches (m_kindBits is a mask), kept sorted. Maintained on node add/remove;
        // instanceRef is fixed for the node lifetime so these never go stale.
        // NOTE KI blend is split by m_useOit so each blend pass scans only its own set:
        // blendOit -> OIT sweep (deferred blend), blendForward -> forward "effect" pass
        // (drawBlendedImpl). A blend drawable is also ALPHA, so its alpha-tested part is in
        // the alpha bucket (deferred g-buffer) regardless of which blend sub-bucket it's in.
        struct LayerDrawables {
            std::vector<uint32_t> solid;
            std::vector<uint32_t> alpha;
            std::vector<uint32_t> blendOit;
            std::vector<uint32_t> blendForward;
        };
        // layer index is a small contiguous set (0 = null, ~1..5); fixed array, O(1) indexed
        // (MAX_LAYERS from asset/LayerInfo.h)
        std::array<LayerDrawables, MAX_LAYERS> m_drawablesByLayer;

        // debug/diagnostic: log if any bucketed index is out-of-bounds, stale, mis-kinded, or unsorted
        void validateDrawables() const;

    public:
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
