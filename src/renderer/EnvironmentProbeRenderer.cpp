#include "EnvironmentProbeRenderer.h"

#include <glm/glm.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/Mesh.h"
#include "mesh/MeshInstance.h"
#include "mesh/MeshType.h"

#include "render/DebugContext.h"

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

    m_programId = ProgramRegistry::get().getProgram(SHADER_VOLUME);
    Program::get(m_programId)->prepareRT();
}

void EnvironmentProbeRenderer::render(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    const auto& dbg = render::DebugContext::get();
    if (!dbg.m_showEnvironmentProbe) return;

    auto& nodeRegistry = NodeRegistry::get();

    std::vector<mesh::MeshInstance> meshes;

    for (const auto* node : nodeRegistry.getCachedNodesRT()) {
        if (!node) continue;

        auto* type = node->getType();
        if (!type->m_flags.cubeMap) continue;

        const auto* snapshot = nodeRegistry.getSnapshotRT(node->m_entityIndex);

        //const auto& volume = snapshot->getVolume();
        const auto& pos = snapshot->getWorldPosition();

        glm::mat4 transform{ 1.f };
        transform = glm::scale(
            glm::translate(transform, glm::vec3{ pos }),
            glm::vec3{ 2.5f });

        meshes.emplace_back(
            transform,
            m_mesh,
            m_mesh->getMaterial()->m_registeredIndex,
            0,
            true);
    }

    drawObjects(ctx, targetBuffer, meshes);
}
