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

void MeshRenderer::prepareRT(const PrepareContext& ctx)
{
    {
        m_fallbackMaterial = Material::createMaterial(BasicMaterial::yellow);
        m_fallbackMaterial.registerMaterial();
    }

    m_programId = ProgramRegistry::get().getProgram("g_tex");
    Program::get(m_programId)->prepareRT();

    m_entityIndex = ID_INDEX;
}

void MeshRenderer::drawObjects(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer,
    const std::vector<mesh::MeshInstance>& meshes)
{
    backend::DrawBuffer* drawBuffer = ctx.m_batch->getDrawBuffer();

    // NOTE KI for troubleshooting
    GLint drawFboId = 0, readFboId = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);

    auto* sharedVao = VaoRegistry::get().getSharedPrimitiveVao();
    auto* dynamicVao = VaoRegistry::get().getDynamicPrimitiveVao();
    dynamicVao->clear();

    PrepareContext prepareCtx{ ctx.m_registry };

    std::vector<mesh::InstanceSSBO> instances;
    for (const auto& meshInstance : meshes)
    {
        auto* mesh = meshInstance.m_mesh.get();

        if (meshInstance.m_shared) {
            mesh->setupVAO(sharedVao, true);
        }
        else {
            mesh->setupVAO(dynamicVao, false);
        }

        auto& instance = instances.emplace_back();
        // NOTE KI null entity/mesh are supposed to have ID mat model matrices
        instance.setTransform(meshInstance.m_transform);
        instance.u_entityIndex = m_entityIndex;
        instance.u_materialIndex = meshInstance.m_materialIndex;
        if (instance.u_materialIndex < 0) {
            instance.u_materialIndex = m_fallbackMaterial.m_registeredIndex;
        }
    }

    sharedVao->updateRT();
    dynamicVao->updateRT();

    targetBuffer->bind(ctx);

    drawBuffer->sendInstanceIndeces(instances);

    ctx.m_state.setDepthFunc(GL_LEQUAL);
    ctx.m_state.setDepthMask(GL_FALSE);

    int baseInstance = 0;
    for (auto& meshInstance : meshes)
    {
        const auto& mesh = meshInstance.m_mesh;
        const auto* primitiveMesh = dynamic_cast<mesh::PrimitiveMesh*>(mesh.get());

        backend::DrawOptions drawOptions;
        {
            drawOptions.m_mode = mesh->getDrawMode();
            drawOptions.m_type = backend::DrawOptions::Type::elements;
            drawOptions.m_solid = true;
            drawOptions.m_wireframe = true;
            drawOptions.m_renderBack = primitiveMesh
                ? (primitiveMesh->m_type == mesh::PrimitiveType::plane || primitiveMesh->m_type == mesh::PrimitiveType::height_field)
                : false;
        }

        backend::DrawRange drawRange{
                drawOptions,
                static_cast<ki::vao_id>(*mesh->getVAO()),
                m_programId,
        };

        backend::gl::DrawIndirectCommand indirect{};
        {
            backend::gl::DrawElementsIndirectCommand& cmd = indirect.element;

            //cmd.u_instanceCount = m_frustumGPU ? 0 : 1;
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
