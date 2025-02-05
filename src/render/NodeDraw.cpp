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
    {}

    NodeDraw::~NodeDraw()
    {
    }

    void NodeDraw::prepareRT(
        const PrepareContext& ctx)
    {
        const auto& assets = ctx.m_assets;
        auto& registry = ctx.m_registry;

        m_passDeferred->prepare(ctx);
        m_passOit->prepare(ctx);
        m_passForward->prepare(ctx);
        m_passDecal->prepare(ctx);
        m_passParticle->prepare(ctx);
        m_passEffect->prepare(ctx);
        m_passFog->prepare(ctx);
        m_passBloom->prepare(ctx);
        m_passSkybox->prepare(ctx);
        m_passDebug->prepare(ctx);
        m_passCopy->prepare(ctx);

        m_timeElapsedQuery.create();

        m_glUseInvalidate = assets.glUseInvalidate;
    }

    void NodeDraw::updateRT(const UpdateViewContext& ctx)
    {
        m_passDeferred->updateRT(ctx);
        m_passOit->updateRT(ctx, m_passDeferred.get());
        m_passForward->updateRT(ctx);
        m_passDecal->updateRT(ctx);
        m_passParticle->updateRT(ctx);
        m_passEffect->updateRT(ctx);
        m_passFog->updateRT(ctx);
        m_passBloom->updateRT(ctx);
        m_passSkybox->updateRT(ctx);
        m_passDebug->updateRT(ctx);
        m_passCopy->updateRT(ctx);
    }

    void NodeDraw::drawNodes(
        const RenderContext& ctx,
        const DrawContext& drawContext,
        FrameBuffer* dstBuffer)
    {
        PassContext passContext;

        {
            m_passDeferred->initRender(ctx);
            m_passOit->initRender(ctx);
            m_passForward->initRender(ctx);
            m_passDecal->initRender(ctx);
            m_passParticle->initRender(ctx);
            m_passEffect->initRender(ctx);
            m_passFog->initRender(ctx);
            m_passOit->initRender(ctx);
            m_passBloom->initRender(ctx);
            m_passSkybox->initRender(ctx);
            m_passDebug->initRender(ctx);
            m_passCopy->initRender(ctx);
        }

        // drawing
        {
            passContext = m_passDeferred->preDepth(ctx, drawContext, passContext);
            passContext = m_passDeferred->render(ctx, drawContext, passContext);
            passContext = m_passDeferred->combine(ctx, drawContext, passContext);

            passContext = m_passForward->render(ctx, drawContext, passContext);

            passContext = m_passSkybox->render(ctx, drawContext, passContext);
            passContext = m_passDecal->render(ctx, drawContext, passContext);
            passContext = m_passParticle->render(ctx, drawContext, passContext);
            passContext = m_passOit->render(ctx, drawContext, passContext);
            passContext = m_passEffect->render(ctx, drawContext, passContext);
        }

        // screeenspace
        {
            passContext = m_passFog->render(ctx, drawContext, passContext);
            passContext = m_passOit->blend(ctx, drawContext, passContext);
            passContext = m_passBloom->render(ctx, drawContext, passContext);
        }

        // post steps
        {
            passContext = m_passDebug->render(ctx, drawContext, passContext);
            passContext = m_passCopy->copy(ctx, drawContext, passContext, dstBuffer);
        }

        passCleanup(ctx);
    }

    void NodeDraw::passCleanup(
        const RenderContext& ctx)
    {
        // pass 12 - cleanup
        if (!m_glUseInvalidate) return;

        m_passDeferred->cleanup(ctx);
        //m_passDeferredCombine->cleanup(ctx);
        //m_passForward->cleanup(ctx);
        //m_passDecal->cleanup(ctx);
        //m_passParticle->cleanup(ctx);
        //m_passEffect->cleanup(ctx);
        //m_passFog->cleanup(ctx);
        m_passOit->cleanup(ctx);
        //m_passBloom->cleanup(ctx);
        //m_passSkybox->cleanup(ctx);
        //m_passDebug->cleanup(ctx);
        //m_passCopy->cleanup(ctx);
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
