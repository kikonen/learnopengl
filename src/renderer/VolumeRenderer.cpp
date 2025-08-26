#include "VolumeRenderer.h"

#include <glm/glm.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/Mesh.h"
#include "mesh/MeshInstance.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/SelectionRegistry.h"


VolumeRenderer::VolumeRenderer() = default;
VolumeRenderer::~VolumeRenderer() = default;

void VolumeRenderer::prepareRT(const PrepareContext& ctx)
{
    MeshRenderer::prepareRT(ctx);

    auto generator = mesh::PrimitiveGenerator::sphere();
    generator.name = "<volume>";
    generator.radius = 1.f;
    generator.slices = 12;
    generator.segments = { 8, 0, 0 };
    m_mesh = generator.create();

    {
        auto material = Material::createMaterial(BasicMaterial::highlight);
        material.m_name = "volume";
        material.kd = glm::vec4(0.8f, 0.8f, 0.f, 1.f);
        material.registerMaterial();
        m_mesh->setMaterial(&material);
    }

    m_programId = ProgramRegistry::get().getProgram(SHADER_VOLUME);
}

void VolumeRenderer::render(
    const RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    const auto& dbg = debug::DebugContext::get();
    if (!(dbg.m_showVolume || dbg.m_showSelectionVolume)) return;

    auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;
    auto& selectionRegistry = *ctx.m_registry->m_selectionRegistry;

    std::vector<mesh::MeshInstance> meshes;

    for (const auto* node : nodeRegistry.getCachedNodesRT()) {
        if (!node) continue;
        if (!dbg.m_showVolume && !selectionRegistry.isSelected(node->toHandle())) continue;

        const auto* snapshot = nodeRegistry.getSnapshotRT(node->m_entityIndex);

        const auto& volume = snapshot->getVolume();
        const auto& pos = snapshot->getWorldPosition();

        const auto transform = glm::translate(glm::mat4{ 1.f }, glm::vec3{ volume }) *
            glm::toMat4(util::degreesToQuat(glm::vec3{90, 0, 0})) *
            glm::scale(glm::mat4{ 1.f }, glm::vec3{ volume.w });

        backend::DrawOptions drawOptions;
        {
            drawOptions.m_mode = m_mesh->getDrawMode();
            drawOptions.m_type = backend::DrawOptions::Type::elements;
            drawOptions.m_lineMode = true;
            drawOptions.m_renderBack = true;
        }

        meshes.emplace_back(
            transform,
            m_mesh,
            drawOptions,
            m_mesh->getMaterial()->m_registeredIndex,
            m_programId,
            true);
    }

    drawObjects(ctx, targetBuffer, meshes);
}
