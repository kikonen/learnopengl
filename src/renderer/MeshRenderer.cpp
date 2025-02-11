#include "MeshRenderer.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/DebugContext.h"
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
    {
        m_fallbackMaterial = Material::createMaterial(BasicMaterial::yellow);
        m_fallbackMaterial.registerMaterial();
    }

    m_programId = ProgramRegistry::get().getProgram("g_tex");

    m_entityIndex = ID_INDEX;
}

void MeshRenderer::drawObjects(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer,
    const std::vector<mesh::MeshInstance>& meshes)
{
    if (meshes.empty()) return;

    backend::DrawBuffer* drawBuffer = ctx.m_batch->getDrawBuffer();

    // NOTE KI for troubleshooting
    //GLint drawFboId = 0, readFboId = 0;
    //glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
    //glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);

    auto* sharedVao = VaoRegistry::get().getSharedPrimitiveVao();
    auto* dynamicVao = VaoRegistry::get().getDynamicPrimitiveVao();
    dynamicVao->clear();

    m_instances.clear();
    m_instances.reserve(meshes.size());

    for (const auto& meshInstance : meshes)
    {
        auto* mesh = meshInstance.m_mesh.get();

        if (meshInstance.m_shared) {
            mesh->setupVAO(sharedVao, true);
        }
        else {
            mesh->setupVAO(dynamicVao, false);
        }

        auto& instance = m_instances.emplace_back();
        // NOTE KI null entity/mesh are supposed to have ID mat model matrices
        instance.setTransform(
            meshInstance.m_transformMatrixRow0,
            meshInstance.m_transformMatrixRow1,
            meshInstance.m_transformMatrixRow2);
        instance.u_entityIndex = m_entityIndex;
        instance.u_materialIndex = meshInstance.m_materialIndex;
        if (instance.u_materialIndex < 0) {
            instance.u_materialIndex = m_fallbackMaterial.m_registeredIndex;
        }
    }

    sharedVao->updateRT();
    dynamicVao->updateRT();

    targetBuffer->bind(ctx);

    drawBuffer->sendInstanceIndeces(m_instances);

    ctx.m_state.setDepthFunc(GL_LESS);
    ctx.m_state.setDepthMask(GL_TRUE);

    int baseInstance = 0;
    for (auto& meshInstance : meshes)
    {
        const auto* mesh = meshInstance.m_mesh.get();

        backend::MultiDrawRange drawRange{
            meshInstance.m_drawOptions,
            static_cast<ki::vao_id>(*mesh->getVAO()),
            meshInstance.m_programId > 0 ? meshInstance.m_programId : m_programId,
        };

        backend::gl::DrawIndirectCommand indirect{};
        {
            backend::gl::DrawElementsIndirectCommand& cmd = indirect.element;

            cmd.u_instanceCount = 1;
            cmd.u_baseInstance = baseInstance;

            cmd.u_baseVertex = mesh->getBaseVertex();
            cmd.u_firstIndex = mesh->getBaseIndex();
            cmd.u_count = mesh->getIndexCount();
        }

        drawBuffer->send(drawRange, indirect);

        baseInstance++;
    }
    drawBuffer->flush();
    drawBuffer->drawPending(false);

    ctx.m_state.setDepthFunc(ctx.m_depthFunc);
    ctx.m_state.setDepthMask(GL_TRUE);
}
