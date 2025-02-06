#include "NodeDraw.h"

#include "asset/Assets.h"

#include "kigl/GLState.h"

#include "pool/TypeHandle.h"

#include "mesh/Mesh.h"
#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "model/Node.h"

#include "engine/PrepareContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "render/FrameBuffer.h"
#include "render/RenderContext.h"
#include "render/DebugContext.h"
#include "render/CollectionRender.h"

#include "render/PassDeferred.h"
#include "render/PassForward.h"
#include "render/PassDecal.h"
#include "render/PassParticle.h"
#include "render/PassEffect.h"
#include "render/PassFog.h"
#include "render/PassOit.h"
#include "render/PassBloom.h"
#include "render/PassSkybox.h"
#include "render/PassDebug.h"
#include "render/PassCopy.h"

#include "size.h"

namespace {
    const std::vector<pool::NodeHandle> EMPTY_NODE_LIST;

    const ki::program_id NULL_PROGRAM_ID = 0;
}

namespace render {
    NodeDraw::NodeDraw()
        : m_passDeferred{ std::make_unique<render::PassDeferred>()},
        m_passOit{ std::make_unique<render::PassOit>() },
        m_passForward{ std::make_unique<render::PassForward>() },
        m_passDecal{ std::make_unique<render::PassDecal>() },
        m_passParticle{ std::make_unique<render::PassParticle>() },
        m_passEffect{ std::make_unique<render::PassEffect>() },
        m_passFog{ std::make_unique<render::PassFog>() },
        m_passBloom{ std::make_unique<render::PassBloom>() },
        m_passSkybox{ std::make_unique<render::PassSkybox>() },
        m_passDebug{ std::make_unique<render::PassDebug>() },
        m_passCopy{ std::make_unique<render::PassCopy>() }
    {
        //m_pipeline.m_forward = false;
    }

    NodeDraw::~NodeDraw()
    {
    }

    void NodeDraw::prepareRT(
        const PrepareContext& ctx)
    {
        const auto& assets = ctx.m_assets;
        auto& registry = ctx.m_registry;

        if (m_pipeline.m_deferred) m_passDeferred->prepare(ctx);
        if (m_pipeline.m_oit) m_passOit->prepare(ctx);
        if (m_pipeline.m_forward) m_passForward->prepare(ctx);
        if (m_pipeline.m_decal) m_passDecal->prepare(ctx);
        if (m_pipeline.m_particle) m_passParticle->prepare(ctx);
        if (m_pipeline.m_effect) m_passEffect->prepare(ctx);
        if (m_pipeline.m_fog) m_passFog->prepare(ctx);
        if (m_pipeline.m_bloom) m_passBloom->prepare(ctx);
        if (m_pipeline.m_skybox) m_passSkybox->prepare(ctx);
        if (m_pipeline.m_debug) m_passDebug->prepare(ctx);
        if (m_pipeline.m_copy) m_passCopy->prepare(ctx);

        m_timeElapsedQuery.create();

        m_glUseInvalidate = assets.glUseInvalidate;
    }

    void NodeDraw::updateRT(const UpdateViewContext& ctx, float bufferScale)
    {
        if (m_pipeline.m_deferred) m_passDeferred->updateRT(ctx, bufferScale);
        if (m_pipeline.m_oit) m_passOit->updateRT(ctx, m_passDeferred.get(), bufferScale);
        if (m_pipeline.m_forward) m_passForward->updateRT(ctx, bufferScale);
        if (m_pipeline.m_decal) m_passDecal->updateRT(ctx, bufferScale);
        if (m_pipeline.m_particle) m_passParticle->updateRT(ctx, bufferScale);
        if (m_pipeline.m_effect) m_passEffect->updateRT(ctx, bufferScale);
        if (m_pipeline.m_fog) m_passFog->updateRT(ctx, bufferScale);
        if (m_pipeline.m_bloom) m_passBloom->updateRT(ctx, bufferScale);
        if (m_pipeline.m_skybox) m_passSkybox->updateRT(ctx, bufferScale);
        if (m_pipeline.m_debug) m_passDebug->updateRT(ctx, bufferScale);
        if (m_pipeline.m_copy) m_passCopy->updateRT(ctx, bufferScale);
    }

    void NodeDraw::drawNodes(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        FrameBuffer* dstBuffer)
    {
        PassContext passContext;

        {
            if (m_pipeline.m_deferred) m_passDeferred->initRender(ctx);
            if (m_pipeline.m_oit) m_passOit->initRender(ctx);
            if (m_pipeline.m_forward) m_passForward->initRender(ctx);
            if (m_pipeline.m_decal) m_passDecal->initRender(ctx);
            if (m_pipeline.m_particle) m_passParticle->initRender(ctx);
            if (m_pipeline.m_effect) m_passEffect->initRender(ctx);
            if (m_pipeline.m_fog) m_passFog->initRender(ctx);
            if (m_pipeline.m_oit) m_passOit->initRender(ctx);
            if (m_pipeline.m_bloom) m_passBloom->initRender(ctx);
            if (m_pipeline.m_skybox) m_passSkybox->initRender(ctx);
            if (m_pipeline.m_debug) m_passDebug->initRender(ctx);
            if (m_pipeline.m_copy) m_passCopy->initRender(ctx);
        }

        // drawing
        {
            if (m_pipeline.m_preDepth)
                passContext = m_passDeferred->preDepth(ctx, drawContext, passContext);
            if (m_pipeline.m_deferred)
                passContext = m_passDeferred->render(ctx, drawContext, passContext);
            if (m_pipeline.m_deferred)
                passContext = m_passDeferred->combine(ctx, drawContext, passContext);

            if (m_pipeline.m_forward)
                passContext = m_passForward->render(ctx, drawContext, passContext);

            if (m_pipeline.m_skybox)
                passContext = m_passSkybox->render(ctx, drawContext, passContext);
            if (m_pipeline.m_decal)
                passContext = m_passDecal->render(ctx, drawContext, passContext);
            if (m_pipeline.m_particle)
                passContext = m_passParticle->render(ctx, drawContext, passContext);
            if (m_pipeline.m_oit)
                passContext = m_passOit->render(ctx, drawContext, passContext);
            if (m_pipeline.m_effect)
                passContext = m_passEffect->render(ctx, drawContext, passContext);
        }

        // screeenspace
        {
            if (m_pipeline.m_fog)
                passContext = m_passFog->render(ctx, drawContext, passContext);
            if (m_pipeline.m_oit)
                passContext = m_passOit->blend(ctx, drawContext, passContext);
            if (m_pipeline.m_bloom)
                passContext = m_passBloom->render(ctx, drawContext, passContext);
        }

        // post steps
        {
            if (m_pipeline.m_debug)
                passContext = m_passDebug->render(ctx, drawContext, passContext);
            if (m_pipeline.m_copy)
                passContext = m_passCopy->copy(ctx, drawContext, passContext, dstBuffer);
        }

        passCleanup(ctx);
    }

    void NodeDraw::passCleanup(
        const RenderContext& ctx)
    {
        // pass 12 - cleanup
        if (!m_glUseInvalidate) return;

        if (m_pipeline.m_deferred) m_passDeferred->cleanup(ctx);
        //if (m_pipeline.m_forward) m_passForward->cleanup(ctx);
        //if (m_pipeline.m_decal) m_passDecal->cleanup(ctx);
        //if (m_pipeline.m_particle) m_passParticle->cleanup(ctx);
        //if (m_pipeline.m_effeect) m_passEffect->cleanup(ctx);
        //if (m_pipeline.m_fog) m_passFog->cleanup(ctx);
        if (m_pipeline.m_decal) m_passOit->cleanup(ctx);
        //if (m_pipeline.m_bloom) m_passBloom->cleanup(ctx);
        //if (m_pipeline.m_skybox) m_passSkybox->cleanup(ctx);
        //if (m_pipeline.m_debug) m_passDebug->cleanup(ctx);
        //if (m_pipeline.m_copy) m_passCopy->cleanup(ctx);
    }

    void NodeDraw::drawProgram(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector)
    {
        CollectionRender collectionRender;
        collectionRender.drawProgram(
            ctx,
            programSelector,
            drawContext.typeSelector,
            drawContext.nodeSelector,
            drawContext.kindBits);
    }
}
