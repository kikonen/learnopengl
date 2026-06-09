#include "NodeCollection.h"

#include <algorithm>

#include <fmt/format.h>

#include "util/Log.h"

#include "model/Node.h"

#include "component/CameraComponent.h"
#include "component/Light.h"

#include "render/size.h"
#include "render/DrawableInfo.h"
#include "render/InstanceRegistry.h"

namespace {
    // Insert an ascending, disjoint block of indices into a sorted bucket, keeping it
    // sorted. The block occupies a slot range owned exclusively by one node, so it lands
    // contiguously at a single position (the end during upward-growing allocation => O(1)).
    void insertSortedBlock(std::vector<uint32_t>& bucket, const std::vector<uint32_t>& block)
    {
        if (block.empty()) return;
        const auto pos = std::lower_bound(bucket.begin(), bucket.end(), block.front());
        bucket.insert(pos, block.begin(), block.end());
    }
}

namespace render {
    NodeCollection::NodeCollection() = default;

    NodeCollection::~NodeCollection()
    {
        clear();
    }

    void NodeCollection::clear()
    {
        for (auto& ld : m_drawablesByLayer) {
            ld.solid.clear();
            ld.alpha.clear();
            ld.blendOit.clear();
            ld.blendForward.clear();
        }

        m_waterNodes.clear();
        m_mirrorNodes.clear();
        m_cubeMapNodes.clear();

        m_activeCameraNode.reset();
        m_cameraNodes.clear();

        m_dirLightNodes.clear();
        m_pointLightNodes.clear();
        m_spotLightNodes.clear();
    }

    void NodeCollection::updateRT(const UpdateContext& ctx)
    {
        for (auto& handle : m_cameraNodes) {
            auto* node = handle.toNode();
            if (!node) continue;
            node->m_camera->updateRT(ctx, *node);
        }
        for (auto& handle : m_pointLightNodes) {
            auto* node = handle.toNode();
            if (!node) continue;
            node->m_light->updateRT(ctx, *node);
        }
        for (auto& handle : m_spotLightNodes) {
            auto* node = handle.toNode();
            if (!node) continue;
            node->m_light->updateRT(ctx, *node);
        }
        for (auto& handle : m_dirLightNodes) {
            auto* node = handle.toNode();
            if (!node) continue;
            node->m_light->updateRT(ctx, *node);
        }
    }

    void NodeCollection::handleNodeAdded(model::Node* node)
    {
        if (!node) return;
        auto nodeHandle = node->toHandle();

        if (!node->m_typeFlags.invisible) {
            // per-kind drawable-index buckets (non-invisible nodes only); the draw sweep
            // iterates these directly, no per-kind node lists needed
            addDrawables(node);
        }

        if (node->m_typeFlags.water) {
            m_waterNodes.push_back(nodeHandle);
        }
        if (node->m_typeFlags.mirror) {
            m_mirrorNodes.push_back(nodeHandle);
        }
        if (node->m_typeFlags.cubeMap) {
            m_cubeMapNodes.push_back(nodeHandle);
        }

        if (node->m_camera) {
            m_cameraNodes.push_back(nodeHandle);
            if (!m_activeCameraNode && node->m_camera->isDefault()) {
                setActiveCameraNode(nodeHandle);
            }
        }

        if (node->m_light) {
            Light* light = node->m_light.get();

            if (light->isDirectional()) {
                m_dirLightNodes.push_back(nodeHandle);
            }
            else if (light->isPoint()) {
                m_pointLightNodes.push_back(nodeHandle);
            }
            else if (light->isSpot()) {
                m_spotLightNodes.push_back(nodeHandle);
            }
        }
    }

    void NodeCollection::handleNodeRemoved(model::Node* node)
    {
        if (!node) return;
        auto nodeHandle = node->toHandle();

        // NOTE KI must run before Node::releaseInstances resets m_instanceRef
        // (Scene::handleNodeRemoved is ordered collection-first for this reason)
        removeDrawables(node);

        nodeHandle.removeFrom(m_waterNodes);
        nodeHandle.removeFrom(m_mirrorNodes);
        nodeHandle.removeFrom(m_cubeMapNodes);

        nodeHandle.removeFrom(m_cameraNodes);

        nodeHandle.removeFrom(m_dirLightNodes);
        nodeHandle.removeFrom(m_pointLightNodes);
        nodeHandle.removeFrom(m_spotLightNodes);
    }

    void NodeCollection::addDrawables(model::Node* node)
    {
        const auto ref = node->getInstanceRef();
        if (ref.empty()) return;

        const auto& drawables = InstanceRegistry::get().getRange(ref);

        // collect this node's matching indices (ascending) per kind, then insert as one
        // sorted block so the buckets stay sorted for sequential sweep access
        std::vector<uint32_t> solid, alpha, blendOit, blendForward;
        for (uint32_t i = 0; i < drawables.size(); i++) {
            const auto& drawable = drawables[i];
            if (drawable.entityIndex == 0) continue;

            const auto& drawOptions = drawable.drawOptions;
            if (drawOptions.m_type == backend::DrawOptions::Type::none) continue;

            const uint32_t index = ref.offset + i;

            // kinds are exclusive (SOLID | ALPHA | ALPHA+BLEND); the invariant is asserted at
            // the source in LodMesh::setupDrawOptions. a BLEND drawable is also ALPHA, so it
            // lands in both the alpha bucket (g-buffer alpha-tested part) and one blend
            // sub-bucket (m_useOit -> OIT pass, else forward "effect" pass).
            if (drawOptions.isKind(render::KIND_SOLID)) solid.push_back(index);
            if (drawOptions.isKind(render::KIND_ALPHA)) alpha.push_back(index);
            if (drawOptions.isKind(render::KIND_BLEND)) {
                if (drawOptions.m_useOit) blendOit.push_back(index);
                else blendForward.push_back(index);
            }
        }

        if (node->m_layer >= MAX_LAYERS) {
            KI_ERROR_OUT(fmt::format("DRAWABLE_BUCKET: layer index {} >= MAX_LAYERS", node->m_layer));
            return;
        }

        auto& ld = m_drawablesByLayer[node->m_layer];
        insertSortedBlock(ld.solid, solid);
        insertSortedBlock(ld.alpha, alpha);
        insertSortedBlock(ld.blendOit, blendOit);
        insertSortedBlock(ld.blendForward, blendForward);
    }

    void NodeCollection::removeDrawables(model::Node* node)
    {
        const auto ref = node->getInstanceRef();
        if (ref.empty()) return;

        if (node->m_layer >= MAX_LAYERS) return;

        const uint32_t lo = ref.offset;
        const uint32_t hi = ref.offset + ref.size;

        const auto inRange = [lo, hi](uint32_t index) {
            return index >= lo && index < hi;
        };

        auto& ld = m_drawablesByLayer[node->m_layer];
        std::erase_if(ld.solid, inRange);
        std::erase_if(ld.alpha, inRange);
        std::erase_if(ld.blendOit, inRange);
        std::erase_if(ld.blendForward, inRange);
    }

    void NodeCollection::validateDrawables() const
    {
        // NOTE KI logs (works in release, unlike assert) — on-demand consistency check
        const auto& reg = InstanceRegistry::get();

        // wantOit: -1 = don't care, 0 = require !useOit, 1 = require useOit
        const auto check = [&reg](const char* name, uint8_t layer, const std::vector<uint32_t>& bucket, uint8_t kind, int wantOit) {
            const bool sorted = std::is_sorted(bucket.begin(), bucket.end());
            size_t stale = 0;
            size_t wrongKind = 0;
            size_t wrongOit = 0;
            size_t outOfBounds = 0;
            for (const uint32_t index : bucket) {
                const auto span = reg.getRange({ index, 1 });
                if (span.empty()) { outOfBounds++; continue; }
                const auto& d = span[0];
                if (d.entityIndex == 0) stale++;
                if (!d.drawOptions.isKind(kind)) wrongKind++;
                if (wantOit >= 0 && d.drawOptions.m_useOit != (wantOit != 0)) wrongOit++;
            }

            if (!sorted || stale || wrongKind || wrongOit || outOfBounds) {
                KI_ERROR_OUT(fmt::format(
                    "DRAWABLE_BUCKET INVALID: layer={} {} size={} sorted={} stale={} wrongKind={} wrongOit={} oob={}",
                    layer, name, bucket.size(), sorted, stale, wrongKind, wrongOit, outOfBounds));
            }
        };

        for (uint8_t layer = 0; layer < MAX_LAYERS; layer++) {
            const auto& ld = m_drawablesByLayer[layer];
            check("solid", layer, ld.solid, render::KIND_SOLID, -1);
            check("alpha", layer, ld.alpha, render::KIND_ALPHA, -1);
            check("blendOit", layer, ld.blendOit, render::KIND_BLEND, 1);
            check("blendForward", layer, ld.blendForward, render::KIND_BLEND, 0);
        }
    }

    void NodeCollection::setActiveCameraNode(pool::NodeHandle nodeHandle)
    {
        if (!nodeHandle) {
            nodeHandle = findDefaultCameraNode();
        }

        auto* node = nodeHandle.toNode();
        if (!node) return;
        if (!node->m_camera) return;

        m_activeCameraNode = nodeHandle;
    }

    pool::NodeHandle NodeCollection::getNextCameraNode(
        pool::NodeHandle srcNode,
        int offset) const noexcept
    {
        int index = 0;
        int size = static_cast<int>(m_cameraNodes.size());
        for (int i = 0; i < size; i++) {
            if (m_cameraNodes[i] == srcNode) {
                index = std::max(0, (i + offset) % size);
                break;
            }
        }
        return m_cameraNodes[index];
    }

    pool::NodeHandle NodeCollection::findDefaultCameraNode() const
    {
        const auto& it = std::find_if(
            m_cameraNodes.begin(),
            m_cameraNodes.end(),
            [](pool::NodeHandle handle) {
                auto* node = handle.toNode();
                return node && node->m_camera->isDefault();
            });
        return it != m_cameraNodes.end() ? *it : pool::NodeHandle::NULL_HANDLE;
    }
}
