#include "PhysicsRenderer.h"

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

void PhysicsRenderer::prepareRT(const PrepareContext& ctx)
{
    {
        m_fallbackMaterial = Material::createMaterial(BasicMaterial::yellow);
        m_fallbackMaterial.registerMaterial();
    }

    m_objectProgramId = ProgramRegistry::get().getProgram("g_tex");
    Program::get(m_objectProgramId)->prepareRT();

    //m_entityIndex = EntityRegistry::get().registerEntity();
    //{
    //    auto* entity = EntityRegistry::get().modifyEntity(m_entityIndex, true);
    //    entity->setModelMatrix(glm::mat4(1.f), true, true);
    //}
}

void PhysicsRenderer::render(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    drawObjects(ctx, targetBuffer);
}

void PhysicsRenderer::drawObjects(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    const auto& dbg = render::DebugContext::get();

    if (!dbg.m_physicsShowObjects) return;

    backend::DrawBuffer* drawBuffer = ctx.m_batch->getDrawBuffer();

    // NOTE KI for troubleshooting
    GLint drawFboId = 0, readFboId = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);

    auto meshes = dbg.m_physicsMeshes;

    if (!meshes || meshes->empty()) return;

    auto* sharedVao = VaoRegistry::get().getSharedPrimitiveVao();
    auto* dynamicVao = VaoRegistry::get().getDynamicPrimitiveVao();
    dynamicVao->clear();

    PrepareContext prepareCtx{ ctx.m_registry };

    std::vector<mesh::InstanceSSBO> instances;
    for (auto& meshInstance : *meshes)
    {
        auto& mesh = meshInstance.m_mesh;

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

    //ctx.m_state.setEnabled(GL_DEPTH_TEST, false);
    ctx.m_state.setDepthFunc(GL_LEQUAL);
    ctx.m_state.setDepthMask(GL_FALSE);

    int baseInstance = 0;
    for (auto& meshInstance : *meshes)
    {
        auto& mesh = meshInstance.m_mesh;
        auto* primitiveMesh = dynamic_cast<mesh::PrimitiveMesh*>(mesh.get());

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
                mesh->getVAO(),
                m_objectProgramId,
                drawOptions,
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

    //ctx.m_state.setEnabled(GL_DEPTH_TEST, true);
    ctx.m_state.setDepthFunc(ctx.m_depthFunc);
    ctx.m_state.setDepthMask(GL_TRUE);
}
