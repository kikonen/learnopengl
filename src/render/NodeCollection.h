#pragma once

#include <vector>
#include <map>
#include <array>
#include <cstdint>

#include "asset/LayerInfo.h"

#include "util/BufferReference.h"

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
        // Per-(layer, route, kind) drawable-index buckets (global indices into
        // InstanceRegistry::m_drawables) the sweep iterates. Keyed by node layer so an
        // unbalanced layer (e.g. ui) doesn't scan the main set, then by render route so a
        // route-exclusive pass (forward, g-buffer, OIT) sweeps only its own set. Kept sorted;
        // maintained on node add/remove (instanceRef is fixed for the node lifetime).
        // A drawable lands in every kind it matches (m_kindBits is a mask), so a BLEND drawable
        // (also ALPHA) is in both its route's alpha bucket (alpha-tested part: g-buffer/shadow)
        // and its route's blend bucket (deferred.blend -> OIT, forward.blend -> effect pass).
        struct RouteBuckets {
            std::vector<uint32_t> solid;
            std::vector<uint32_t> alpha;
            std::vector<uint32_t> blend;
        };

        struct LayerDrawables {
            RouteBuckets deferred;   // g-buffer, pre-depth, OIT (deferred.blend)
            RouteBuckets forward;    // forward pass, effect (forward.blend)
            // shadow casters (!noShadow), cross-route, one entry per drawable; swept by the
            // shadow pass so non-casters (e.g. a 1M-instance noShadow generator) aren't streamed.
            std::vector<uint32_t> shadow;

            // Cull groups (volume-sharing runs: a node, or a generator instance's LOD meshes)
            // for THIS layer, so a layer pass culls only its own drawables (a sparse layer or a
            // shadow cascade doesn't frustum-test the whole main set). shadowCullGroups is the
            // caster subset (groups with any !noShadow drawable) for the shadow cull.
            std::vector<util::BufferReference> cullGroups;
            std::vector<util::BufferReference> shadowCullGroups;
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
