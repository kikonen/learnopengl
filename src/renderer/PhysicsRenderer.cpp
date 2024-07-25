#include "PhysicsRenderer.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/DebugContext.h"
#include "render/Batch.h"

#include "kigl/GLState.h"

#include "mesh/Mesh.h"
#include "mesh/PrimitiveMesh.h"
#include "mesh/vao/TexturedVAO.h"
#include "registry/VaoRegistry.h"
#include "backend/DrawBuffer.h"
#include "asset/Program.h"

#include "mesh/TransformRegistry.h"

#include "registry/MaterialRegistry.h"
#include "registry/ProgramRegistry.h"
#include "registry/EntityRegistry.h"

void PhysicsRenderer::prepareRT(const PrepareContext& ctx)
{
    m_fallbackMaterial = Material::createMaterial(BasicMaterial::yellow);
    MaterialRegistry::get().registerMaterial(m_fallbackMaterial);

    m_objectProgram = ProgramRegistry::get().getProgram("g_tex");
    m_objectProgram->prepareRT();

    m_entityIndex = EntityRegistry::get().registerEntity();
    {
        auto* entity = EntityRegistry::get().modifyEntity(m_entityIndex, true);
        entity->setModelMatrix(glm::mat4(1.f), true, true);
    }

    m_meshIndex = mesh::TransformRegistry::get().registerTransform(glm::mat4{ 1.f });
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
    const auto& debugContext = render::DebugContext::get();

    if (!debugContext.m_physicsShowObjects) return;

    backend::DrawBuffer* drawBuffer = ctx.m_batch->getDrawBuffer();

    // NOTE KI for troubleshooting
    GLint drawFboId = 0, readFboId = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);

    auto meshes = debugContext.m_physicsMeshes;

    if (!meshes || meshes->empty()) return;

    auto* vao = VaoRegistry::get().getPrimitiveVao();
    vao->clear();

    PrepareContext prepareCtx{ ctx.m_registry };

    std::vector<mesh::InstanceSSBO> instances;
    for (auto& mesh : *meshes) {
        mesh->setupVAO(vao);

        auto& instance = instances.emplace_back();
        // NOTE KI null entity/mesh are supposed to have ID mat model matrices
        instance.u_entityIndex = m_entityIndex;
        instance.u_meshIndex = m_meshIndex;
        instance.u_materialIndex = mesh->getMaterial().m_registeredIndex;
        if (instance.u_materialIndex < 0) {
            instance.u_materialIndex = m_fallbackMaterial.m_registeredIndex;
        }
    }

    vao->updateRT();

    targetBuffer->bind(ctx);

    drawBuffer->sendInstanceIndeces(instances);

    //ctx.m_state.setEnabled(GL_DEPTH_TEST, false);
    ctx.m_state.setDepthFunc(GL_LEQUAL);

    int baseInstance = 0;
    for (auto& mesh : *meshes)
    {
        backend::DrawOptions drawOptions;
        {
            drawOptions.m_mode = mesh->getDrawMode();
            drawOptions.m_type = backend::DrawOptions::Type::elements;
            drawOptions.m_solid = true;
            drawOptions.m_wireframe = true;
            drawOptions.m_renderBack = false;
        }

        backend::DrawRange drawRange{
                vao->getVAO(),
                m_objectProgram,
                drawOptions };

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
}
