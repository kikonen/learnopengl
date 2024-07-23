#include "PhysicsRenderer.h"

#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/DebugContext.h"
#include "render/Batch.h"

#include "mesh/Mesh.h"
#include "mesh/vao/TexturedVAO.h"
#include "physics/PhysicsEngine.h"
#include "registry/VaoRegistry.h"
#include "backend/DrawBuffer.h"
#include "asset/Program.h"

#include "mesh/TransformRegistry.h"

#include "registry/MaterialRegistry.h"
#include "registry/ProgramRegistry.h"
#include "registry/EntityRegistry.h"

void PhysicsRenderer::prepareRT(const PrepareContext& ctx)
{
    m_objectMaterial = Material::createMaterial(BasicMaterial::white);
    MaterialRegistry::get().registerMaterial(m_objectMaterial);

    m_planeMaterial = Material::createMaterial(BasicMaterial::blue);
    MaterialRegistry::get().registerMaterial(m_planeMaterial);

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
    if (!render::DebugContext::get().m_physicsShowObjects) return;

    backend::DrawBuffer* drawBuffer = ctx.m_batch->getDrawBuffer();

    // NOTE KI for troubleshooting
    GLint drawFboId = 0, readFboId = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);

    auto meshes = physics::PhysicsEngine::get().getObjectMeshes();

    if (meshes && !meshes->empty()) {
        auto* vao = VaoRegistry::get().getDebugVao();
        vao->clear();

        PrepareContext prepareCtx{ ctx.m_registry };

        std::vector<mesh::InstanceSSBO> instances;
        for (auto& mesh : *meshes) {
            mesh->prepareRTDebug(prepareCtx);

            auto& instance = instances.emplace_back();
            // NOTE KI null entity/mesh are supposed to have ID mat model matrices
            instance.u_entityIndex = m_entityIndex;
            instance.u_meshIndex = m_meshIndex;
            instance.u_materialIndex = m_objectMaterial.m_registeredIndex;
            if (mesh->m_alias == "plane") {
                instance.u_materialIndex = m_planeMaterial.m_registeredIndex;
            }
        }

        vao->updateRT();

        targetBuffer->bind(ctx);

        drawBuffer->sendInstanceIndeces(instances);

        int baseInstance = 0;
        for (auto& mesh : *meshes)
        {
            backend::DrawOptions drawOptions;
            {
                drawOptions.m_mode = backend::DrawOptions::Mode::triangles;
                drawOptions.m_type = backend::DrawOptions::Type::elements;
                drawOptions.m_solid = true;
                drawOptions.m_wireframe = true;
                //drawOptions.m_renderBack = true;
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
    }
}
