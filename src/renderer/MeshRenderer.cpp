#include "MeshRenderer.h"

#include "asset/Assets.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "debug/DebugContext.h"
#include "render/Batch.h"

#include "kigl/GLState.h"

#include "mesh/Mesh.h"
#include "mesh/PrimitiveMesh.h"
#include "mesh/MeshInstance.h"
#include "mesh/vao/TexturedVAO.h"

#include "registry/VaoRegistry.h"
#include "backend/DrawBuffer.h"

#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

namespace {
    constexpr int ID_INDEX = 1;
}

MeshRenderer::MeshRenderer() = default;
MeshRenderer::~MeshRenderer() = default;

void MeshRenderer::prepareRT(const PrepareContext& ctx)
{
    const auto& assets = Assets::get();

    {
        m_fallbackMaterial = Material::createMaterial(BasicMaterial::yellow);
        m_fallbackMaterial.registerMaterial();
    }

    m_programId = ProgramRegistry::get().getProgram("g_tex");

    m_useFenceDebug = assets.glUseFenceDebug;

    m_entityIndex = ID_INDEX;
}

void MeshRenderer::drawObjects(
    const render::RenderContext& ctx,
    render::FrameBuffer* targetBuffer,
    const std::vector<mesh::MeshInstance>& meshes)
{
    if (meshes.empty()) return;

    auto* batch = ctx.m_batch;

    // NOTE KI for troubleshooting
    //GLint drawFboId = 0, readFboId = 0;
    //glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
    //glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);

    auto* sharedVao = VaoRegistry::get().getSharedPrimitiveVao();
    mesh::TexturedVAO* dynamicVao = nullptr;

    bool hasDynamic = false;
    for (const auto& meshInstance : meshes)
    {
        hasDynamic |= !meshInstance.m_shared;
    }

    if (hasDynamic) {
        m_dynamicVaoIndex = (m_dynamicVaoIndex + 1) % 2;
        dynamicVao = VaoRegistry::get().getDynamicPrimitiveVao(m_dynamicVaoIndex);
        dynamicVao->getFence().waitFence(m_useFenceDebug);
        dynamicVao->clear();
    }

    for (const auto& meshInstance : meshes)
    {
        auto* mesh = meshInstance.m_mesh;

        if (meshInstance.m_shared) {
            mesh->setupVAO(sharedVao, true);
        }
        else {
            mesh->setupVAO(dynamicVao, false);
        }
    }

    sharedVao->updateRT();
    if (dynamicVao) {
        dynamicVao->updateRT();
    }

    targetBuffer->bind(ctx);

    auto& state = ctx.getGLState();
    state.setDepthFunc(GL_LESS);
    state.setDepthMask(GL_TRUE);

    for (auto& meshInstance : meshes)
    {
        batch->addMesh(ctx, render::KIND_SOLID, meshInstance, m_programId, m_entityIndex);
    }
    batch->flush(ctx);

    state.setDepthFunc(ctx.m_depthFunc);
    state.setDepthMask(GL_TRUE);

    if (dynamicVao) {
        dynamicVao->getFence().setFence(m_useFenceDebug);
    }
}
