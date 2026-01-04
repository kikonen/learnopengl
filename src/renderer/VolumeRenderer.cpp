#include "VolumeRenderer.h"

#include <glm/glm.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/Mesh.h"
#include "mesh/MeshInstance.h"
#include "mesh/Transform.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/SelectionRegistry.h"


VolumeRenderer::VolumeRenderer()
    :m_meshFixMatrix{ 1.f }
{
}

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

    // volume is 90 degrees wrong way visulization wise
    m_meshFixRotation = util::degreesToQuat(glm::vec3{ 90, 0, 0 });
    m_meshFixMatrix = glm::toMat4(m_meshFixRotation);

    {
        auto material = Material::createMaterial(BasicMaterial::highlight);
        material.m_name = "volume";
        material.kd = glm::vec4(glm::vec3(245, 150, 66) / 255.f, 1.f);
        material.registerMaterial();
        m_mesh->setMaterial(&material);
    }

    m_programId = ProgramRegistry::get().getProgram(SHADER_VOLUME);
}

void VolumeRenderer::render(
    const render::RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    const auto& dbg = debug::DebugContext::get();
    if (!(dbg.m_showVolume || dbg.m_showSelectionVolume)) return;

    auto& nodeRegistry = *ctx.getRegistry()->m_nodeRegistry;
    auto& selectionRegistry = *ctx.getRegistry()->m_selectionRegistry;

    m_meshes.clear();
    if (dbg.m_showVolume)
    {
        m_meshes.reserve(nodeRegistry.getCachedNodesRT().size());
    }

    for (const auto* node : nodeRegistry.getCachedNodesRT()) {
        if (!node) continue;
        if (node->m_typeFlags.invisible ||
            !node->m_visible ||
            !node->m_alive ||
            node->m_typeFlags.skybox) continue;
        if (!dbg.m_showVolume && !selectionRegistry.isSelected(node->toHandle())) continue;

        if (const auto* generator = node->m_generator.get(); generator)
        {
            const auto& parentState = nodeRegistry.getParentState(node->getEntityIndex());

            for (auto& meshTransform : generator->getTransforms())
            {
                const auto& worldVolume = meshTransform.getWorldVolume();
                const auto& center = worldVolume.getCenter();

                backend::DrawOptions drawOptions;
                {
                    drawOptions.m_mode = m_mesh->getDrawMode();
                    drawOptions.m_type = backend::DrawOptions::Type::elements;
                    drawOptions.m_lineMode = true;
                    drawOptions.m_renderBack = true;
                }

                mesh::Transform transform;
                transform.setPosition(center);
                transform.setRotation(m_meshFixRotation);
                transform.setScale(worldVolume.radius);
                transform.updateMatrix();

                m_meshes.emplace_back(
                    m_mesh.get(),
                    transform.getMatrix(),
                    worldVolume,
                    drawOptions,
                    m_mesh->getMaterial()->m_registeredIndex,
                    m_programId,
                    true);
            }
        }
        else {
            const auto* snapshot = nodeRegistry.getSnapshotRT(node->getEntityIndex());

            if (snapshot) {
                const auto& worldVolume = snapshot->getWorldVolume();
                const auto& center = worldVolume.getCenter();

                mesh::Transform transform;
                transform.setPosition(center);
                transform.setRotation(m_meshFixRotation);
                transform.setScale(worldVolume.radius);
                transform.updateMatrix();

                backend::DrawOptions drawOptions;
                {
                    drawOptions.m_mode = m_mesh->getDrawMode();
                    drawOptions.m_type = backend::DrawOptions::Type::elements;
                    drawOptions.m_lineMode = true;
                    drawOptions.m_renderBack = true;
                }

                auto& meshIntance = m_meshes.emplace_back(
                    m_mesh.get(),
                    transform.getMatrix(),
                    worldVolume,
                    drawOptions,
                    m_mesh->getMaterial()->m_registeredIndex,
                    m_programId,
                    true);
            }
        }
    }

    drawObjects(ctx, targetBuffer, m_meshes);
}
