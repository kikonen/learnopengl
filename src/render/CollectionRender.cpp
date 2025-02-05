#include "CollectionRender.h"

#include "kigl/GLState.h"

#include "model/Node.h"

#include "mesh/MeshType.h"

#include "render/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/Batch.h"
#include "render/NodeCollection.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace render
{
    void CollectionRender::drawProgram(
        const RenderContext& ctx,
        const std::function<ki::program_id(const mesh::LodMesh&)>& programSelector,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        uint8_t kindBits)
    {
        drawNodesImpl(ctx, programSelector, typeSelector, nodeSelector, kindBits);
    }

    bool CollectionRender::drawNodesImpl(
        const RenderContext& ctx,
        const std::function<ki::program_id(const mesh::LodMesh&)>& programSelector,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector,
        const uint8_t kindBits)
    {
        bool rendered{ false };

        auto& collection = *ctx.m_collection;
        auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;

        auto renderTypes = [this, &ctx, &programSelector, &typeSelector, &nodeSelector, &rendered](
            const MeshTypeMap& typeMap,
            unsigned int kind)
            {
                for (const auto& it : typeMap) {
                    auto* type = it.first.m_typeHandle.toType();

                    if (!type->isReady()) continue;
                    if (type->m_layer != ctx.m_layer) continue;
                    if (!typeSelector(type)) continue;

                    auto& batch = ctx.m_batch;

                    for (auto& handle : it.second) {
                        auto* node = handle.toNode();
                        if (!node || !nodeSelector(node)) continue;

                        rendered = true;
                        batch->draw(ctx, type, programSelector, kind, *node);
                    }
                }
            };

        if (kindBits & render::KIND_SOLID) {
            renderTypes(collection.m_solidNodes, render::KIND_SOLID);
        }

        if (kindBits & render::KIND_ALPHA) {
            renderTypes(collection.m_alphaNodes, render::KIND_ALPHA);
        }

        if (kindBits & render::KIND_BLEND) {
            renderTypes(collection.m_blendedNodes, render::KIND_BLEND);
        }

        return rendered;
    }

    void CollectionRender::drawBlendedImpl(
        const RenderContext& ctx,
        const std::function<bool(const mesh::MeshType*)>& typeSelector,
        const std::function<bool(const Node*)>& nodeSelector)
    {
        auto& collection = *ctx.m_collection;

        if (collection.m_blendedNodes.empty()) return;

        const glm::vec3& eyePos = ctx.m_camera->getWorldPosition();

        // TODO KI discards nodes if *same* distance
        std::map<float, Node*> sorted;
        for (const auto& map : collection.m_blendedNodes) {
            auto* type = map.first.m_typeHandle.toType();

            if (!type->isReady()) continue;
            if (type->m_layer != ctx.m_layer) continue;
            if (!typeSelector(type)) continue;

            for (const auto& handle : map.second) {
                auto* node = handle.toNode();
                if (!node || !nodeSelector(node)) continue;

                const auto* snapshot = node->getSnapshotRT();
                if (!snapshot) continue;

                const auto& pos = snapshot->getWorldPosition();
                const float dist2 = glm::distance2(eyePos, pos);
                sorted[dist2] = node;
            }
        }

        if (!sorted.empty()) {
            //glMemoryBarrier(GL_ALL_BARRIER_BITS);
            glFlush();
            //glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);
            //glFinish();
        }

        // NOTE KI blending is *NOT* optimal program / nodetypw wise due to depth sorting
        // NOTE KI order = from furthest away to nearest
        for (std::map<float, Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
            auto* node = it->second;
            auto* type = node->m_typeHandle.toType();

            ctx.m_batch->draw(
                ctx,
                type,
                [this](const mesh::LodMesh& lodMesh) { return lodMesh.m_programId; },
                render::KIND_BLEND,
                *node);
        }

        // TODO KI if no flush here then render order of blended nodes is incorrect
        //ctx.m_batch->flush(ctx);
    }
}
