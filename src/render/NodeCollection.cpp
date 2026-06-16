#include "NodeCollection.h"

#include <algorithm>
#include <functional>

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
            for (auto* r : { &ld.deferred, &ld.forward }) {
                r->solid.clear();
                r->alpha.clear();
                r->blend.clear();
            }
            ld.shadow.clear();
            ld.cullGroups.clear();
            ld.shadowCullGroups.clear();
        }

        m_waterNodes.clear();
        m_mirrorNodes.clear();
        m_cubeMapNodes.clear();
        m_environmentProbeNodes.clear();

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
        if (node->m_typeFlags.environmentProbe) {
            m_environmentProbeNodes.push_back(nodeHandle);
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
        nodeHandle.removeFrom(m_environmentProbeNodes);

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

        // collect this node's matching indices (ascending) per (route, kind), then insert as one
        // sorted block so the buckets stay sorted for sequential sweep access
        struct Local { std::vector<uint32_t> solid, alpha, blend; };
        Local deferred, forward;
        std::vector<uint32_t> shadow;   // !noShadow casters (cross-route)

        for (uint32_t i = 0; i < drawables.size(); i++) {
            const auto& drawable = drawables[i];
            if (drawable.entityIndex == 0) continue;

            const auto& drawOptions = drawable.drawOptions;
            if (drawOptions.m_type == backend::DrawOptions::Type::none) continue;

            const uint32_t index = ref.offset + i;

            // route is per drawable (material-derived): deferred -> g-buffer/OIT, else forward.
            // (deferred blend == useOit -> deferred.blend; forward/effect blend -> forward.blend.)
            Local& route = drawOptions.m_useDeferred ? deferred : forward;

            // kinds are exclusive (SOLID | ALPHA | ALPHA+BLEND); invariant asserted at the source
            // in LodMesh::setupDrawOptions. a BLEND drawable is also ALPHA, so it lands in both its
            // route's alpha bucket (alpha-tested part: g-buffer/shadow) and its route's blend bucket.
            if (drawOptions.isKind(render::KIND_SOLID)) route.solid.push_back(index);
            if (drawOptions.isKind(render::KIND_ALPHA)) route.alpha.push_back(index);
            if (drawOptions.isKind(render::KIND_BLEND)) route.blend.push_back(index);

            // shadow casters (cross-route, one entry each); lets the shadow pass skip a huge
            // noShadow set (e.g. a 1M-instance generator) instead of rejecting per drawable.
            if (!drawable.m_flags.noShadow) shadow.push_back(index);
        }

        if (node->m_layer >= MAX_LAYERS) {
            KI_ERROR_OUT(fmt::format("DRAWABLE_BUCKET: layer index {} >= MAX_LAYERS", node->m_layer));
            return;
        }

        auto& ld = m_drawablesByLayer[node->m_layer];
        insertSortedBlock(ld.deferred.solid, deferred.solid);
        insertSortedBlock(ld.deferred.alpha, deferred.alpha);
        insertSortedBlock(ld.deferred.blend, deferred.blend);
        insertSortedBlock(ld.forward.solid, forward.solid);
        insertSortedBlock(ld.forward.alpha, forward.alpha);
        insertSortedBlock(ld.forward.blend, forward.blend);
        insertSortedBlock(ld.shadow, shadow);

        // Cull groups: split this node's contiguous range into volume-sharing runs. stride =
        // meshes-per-instance (whole ref = 1 group for a non-generator node; one group per
        // instance for a generator). A group is a shadow-caster group if any of its drawables
        // casts (!noShadow), so the shadow cull skips a fully-noShadow generator entirely.
        // (Order doesn't matter: the cull writes disjoint ranges via par_unseq.)
        const uint32_t total = ref.size;
        uint32_t stride = static_cast<uint32_t>(node->getEnabledMeshes().size());
        if (stride == 0) stride = total;
        for (uint32_t g = 0; g < total; g += stride) {
            const uint32_t size = std::min(stride, total - g);
            const util::BufferReference group{ ref.offset + g, size };
            ld.cullGroups.push_back(group);

            bool caster = false;
            for (uint32_t k = 0; k < size && !caster; k++) {
                if (!drawables[g + k].m_flags.noShadow) caster = true;
            }
            if (caster) ld.shadowCullGroups.push_back(group);
        }
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
        for (auto* r : { &ld.deferred, &ld.forward }) {
            std::erase_if(r->solid, inRange);
            std::erase_if(r->alpha, inRange);
            std::erase_if(r->blend, inRange);
        }
        std::erase_if(ld.shadow, inRange);

        const auto groupInRange = [lo, hi](const util::BufferReference& g) {
            return g.offset >= lo && g.offset < hi;
        };
        std::erase_if(ld.cullGroups, groupInRange);
        std::erase_if(ld.shadowCullGroups, groupInRange);
    }

    void NodeCollection::validateDrawables() const
    {
        // NOTE KI logs (works in release, unlike assert) — on-demand consistency check
        const auto& reg = InstanceRegistry::get();

        // valid: per-bucket membership predicate (route + kind); a bucket entry must be alive
        // (entityIndex != 0), satisfy the predicate, be in range, and the bucket stays sorted.
        const auto check = [&reg](
            const char* name, uint8_t layer, const std::vector<uint32_t>& bucket,
            const std::function<bool(const DrawableInfo&)>& valid)
        {
            const bool sorted = std::is_sorted(bucket.begin(), bucket.end());
            size_t stale = 0;
            size_t invalid = 0;
            size_t outOfBounds = 0;
            for (const uint32_t index : bucket) {
                const auto span = reg.getRange({ index, 1 });
                if (span.empty()) { outOfBounds++; continue; }
                const auto& d = span[0];
                if (d.entityIndex == 0) { stale++; continue; }
                if (!valid(d)) invalid++;
            }

            if (!sorted || stale || invalid || outOfBounds) {
                KI_ERROR_OUT(fmt::format(
                    "DRAWABLE_BUCKET INVALID: layer={} {} size={} sorted={} stale={} invalid={} oob={}",
                    layer, name, bucket.size(), sorted, stale, invalid, outOfBounds));
            }
        };

        for (uint8_t layer = 0; layer < MAX_LAYERS; layer++) {
            const auto& ld = m_drawablesByLayer[layer];
            check("deferred.solid", layer, ld.deferred.solid,
                [](const DrawableInfo& d) { return d.drawOptions.m_useDeferred && d.drawOptions.isKind(render::KIND_SOLID); });
            check("deferred.alpha", layer, ld.deferred.alpha,
                [](const DrawableInfo& d) { return d.drawOptions.m_useDeferred && d.drawOptions.isKind(render::KIND_ALPHA); });
            check("deferred.blend", layer, ld.deferred.blend,
                [](const DrawableInfo& d) { return d.drawOptions.m_useDeferred && d.drawOptions.isKind(render::KIND_BLEND); });
            check("forward.solid", layer, ld.forward.solid,
                [](const DrawableInfo& d) { return !d.drawOptions.m_useDeferred && d.drawOptions.isKind(render::KIND_SOLID); });
            check("forward.alpha", layer, ld.forward.alpha,
                [](const DrawableInfo& d) { return !d.drawOptions.m_useDeferred && d.drawOptions.isKind(render::KIND_ALPHA); });
            check("forward.blend", layer, ld.forward.blend,
                [](const DrawableInfo& d) { return !d.drawOptions.m_useDeferred && d.drawOptions.isKind(render::KIND_BLEND); });
            check("shadow", layer, ld.shadow,
                [](const DrawableInfo& d) { return !d.m_flags.noShadow; });
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
