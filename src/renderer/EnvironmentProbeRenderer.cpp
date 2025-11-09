#include "EnvironmentProbeRenderer.h"

#include <glm/glm.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/Mesh.h"
#include "mesh/Transform.h"
#include "mesh/MeshInstance.h"

#include "debug/DebugContext.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "registry/NodeRegistry.h"

EnvironmentProbeRenderer::EnvironmentProbeRenderer() = default;
EnvironmentProbeRenderer::~EnvironmentProbeRenderer() = default;

void EnvironmentProbeRenderer::prepareRT(const PrepareContext& ctx)
{
    MeshRenderer::prepareRT(ctx);

    auto generator = mesh::PrimitiveGenerator::sphere();
    generator.name = "<probe>";
    generator.radius = 1.f;
    generator.slices = 12;
    generator.segments = { 8, 0, 0 };
    m_mesh = generator.create();

    {
        auto material = Material::createMaterial(BasicMaterial::highlight);
        material.m_name = "probe";
        material.kd = glm::vec4(0.2f, 0.8f, 0.1f, 1.f);
        material.registerMaterial();
        m_mesh->setMaterial(&material);
    }

    // volume is 90 degrees wrong way visulization wise
    m_meshFixRotation = util::degreesToQuat(glm::vec3{ 90, 0, 0 });
    m_meshFixMatrix = glm::toMat4(m_meshFixRotation);

    m_programId = ProgramRegistry::get().getProgram(SHADER_VOLUME);
}

void EnvironmentProbeRenderer::render(
    const render::RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    const auto& dbg = debug::DebugContext::get();
    if (!dbg.m_showEnvironmentProbe) return;

    auto& nodeRegistry = NodeRegistry::get();

    m_meshes.clear();

    for (const auto* node : nodeRegistry.getCachedNodesRT()) {
        if (!node) continue;

        if (!node->m_typeFlags.cubeMap) continue;

        const auto* snapshot = nodeRegistry.getSnapshotRT(node->getEntityIndex());

        //const auto& volume = snapshot->getVolume();
        const auto& worldPos = snapshot->getWorldPosition();

        //const auto transform = glm::translate(glm::mat4{ 1.f }, glm::vec3{ pos }) *
        //    glm::toMat4(util::degreesToQuat(glm::vec3{ 90, 0, 0 })) *
        //    glm::scale(glm::mat4{ 1.f }, glm::vec3{ 2.5f });

        mesh::Transform transform;
        transform.setPosition(worldPos);
        transform.setRotation(m_meshFixRotation);
        transform.setScale(2.5f);

        transform.updateMatrix();
        transform.updateVolume();

        backend::DrawOptions drawOptions;
        {
            drawOptions.m_mode = m_mesh->getDrawMode();
            drawOptions.m_type = backend::DrawOptions::Type::elements;
            drawOptions.m_lineMode = true;
            drawOptions.m_renderBack = false;
        }

        m_meshes.emplace_back(
            m_mesh.get(),
            transform.getMatrix(),
            drawOptions,
            m_mesh->getMaterial()->m_registeredIndex,
            m_programId,
            true);
    }

    drawObjects(ctx, targetBuffer, m_meshes);
}
