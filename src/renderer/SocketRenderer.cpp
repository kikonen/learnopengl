#include "SocketRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <fmt/format.h>

#include "asset/SphereVolume.h"

#include "backend/DrawOptions.h"

#include "engine/PrepareContext.h"

#include "model/Node.h"
#include "model/NodeType.h"

#include "mesh/generator/PrimitiveGenerator.h"
#include "mesh/Mesh.h"
#include "mesh/MeshInstance.h"
#include "mesh/Transform.h"
#include "mesh/LodMesh.h"
#include "mesh/RegisteredRig.h"

#include "debug/DebugContext.h"

#include "render/RenderContext.h"
#include "render/InstanceRegistry.h"

#include "shader/Shader.h"
#include "shader/Program.h"
#include "shader/ProgramRegistry.h"

#include "animation/AnimationSystem.h"
#include "animation/SocketRegistry.h"
#include "animation/Rig.h"
#include "animation/RigSocket.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace {
    std::shared_ptr<mesh::Mesh> createAxisMesh(
        const std::string& name,
        const glm::vec3& dir,
        float length,
        const glm::vec4& color)
    {
        auto generator = mesh::PrimitiveGenerator::box();
        generator.name = name;
        generator.origin = glm::vec3{ 0.f };
        generator.dir = dir;
        generator.length = length;

        auto mesh = generator.create();

        auto material = Material::createMaterial(BasicMaterial::basic);
        material.m_name = name;
        material.kd = color;
        material.registerMaterial();
        mesh->setMaterial(&material);

        return mesh;
    }
}

SocketRenderer::SocketRenderer()
{
}

SocketRenderer::~SocketRenderer() = default;

void SocketRenderer::prepareRT(const PrepareContext& ctx)
{
    MeshRenderer::prepareRT(ctx);

    m_axisLength = 1.0;
    // Create X, Y, Z axis meshes with distinct colors
    m_axisMeshX = createAxisMesh("<socket_axis_x>", glm::vec3{ 1.f, 0.f, 0.f }, m_axisLength, glm::vec4{ 1.f, 0.f, 0.f, 1.f }); // Red
    m_axisMeshY = createAxisMesh("<socket_axis_y>", glm::vec3{ 0.f, 1.f, 0.f }, m_axisLength, glm::vec4{ 0.f, 1.f, 0.f, 1.f }); // Green
    m_axisMeshZ = createAxisMesh("<socket_axis_z>", glm::vec3{ 0.f, 0.f, 1.f }, m_axisLength, glm::vec4{ 0.f, 0.f, 1.f, 1.f }); // Blue

    m_programId = ProgramRegistry::get().getProgram(SHADER_VOLUME);
}

void SocketRenderer::render(
    const render::RenderContext& ctx,
    render::FrameBuffer* targetBuffer)
{
    const auto& dbg = debug::DebugContext::get();
    if (!dbg.m_animation.m_showSockets) return;

    auto& nodeRegistry = *ctx.getRegistry()->m_nodeRegistry;

    m_meshes.clear();
    m_meshes.reserve(64);  // Reasonable initial capacity

    for (const auto* node : nodeRegistry.getCachedNodesRT()) {
        if (!node) continue;
        if (!node->m_visible || !node->m_alive) continue;

        const auto& registeredRigs = node->getRegisteredRigs();
        if (registeredRigs.empty()) continue;

        const auto* snapshot = node->getSnapshotRT();
        if (!snapshot) continue;

        const auto& nodeModelMatrix = snapshot->getModelMatrix();

        // Find matching LodMesh for baseTransform
        auto* type = node->getType();
        const auto& lodMeshes = type->getLodMeshes();

        for (const auto& registeredRig : registeredRigs) {
            const auto* rig = registeredRig.m_rig;
            if (!rig) continue;

            // Find LodMesh matching this rig to get correct baseTransform
            const glm::mat4* baseTransform = nullptr;
            for (const auto& lodMesh : lodMeshes) {
                if (lodMesh.m_mesh && lodMesh.m_mesh->getRig() == rig) {
                    baseTransform = &lodMesh.m_baseTransform;
                    break;
                }
            }

            // Draw axes for each socket in the rig
            for (const auto& socket : rig->getSockets()) {
                if (socket.m_index < 0) continue;

                // Get socket transform from registry
                uint32_t globalSocketIndex = registeredRig.m_socketRef.offset + socket.m_index;
                const auto& socketTransform = animation::AnimationSystem::get().getSocketTransform(globalSocketIndex);

                // Compute world transform for socket
                // socketTransform is relative to mesh space, need to apply node transform
                glm::mat4 worldSocketTransform;
                if (baseTransform) {
                    worldSocketTransform = nodeModelMatrix * (*baseTransform) * socketTransform;
                } else {
                    worldSocketTransform = nodeModelMatrix * socketTransform;
                }

                // Extract position from socket transform
                glm::vec3 socketPos = glm::vec3(worldSocketTransform[3]);

                backend::DrawOptions drawOptions;
                {
                    drawOptions.m_mode = m_axisMeshX->getDrawMode();
                    drawOptions.m_type = backend::DrawOptions::Type::elements;
                    drawOptions.m_lineMode = true;
                    drawOptions.m_renderBack = true;
                }

                // Add X axis
                m_meshes.emplace_back(
                    m_axisMeshX.get(),
                    worldSocketTransform,
                    SphereVolume{ socketPos, m_axisLength },
                    drawOptions,
                    m_axisMeshX->getMaterial()->m_registeredIndex,
                    m_programId,
                    true);

                // Add Y axis
                m_meshes.emplace_back(
                    m_axisMeshY.get(),
                    worldSocketTransform,
                    SphereVolume{ socketPos, m_axisLength },
                    drawOptions,
                    m_axisMeshY->getMaterial()->m_registeredIndex,
                    m_programId,
                    true);

                // Add Z axis
                m_meshes.emplace_back(
                    m_axisMeshZ.get(),
                    worldSocketTransform,
                    SphereVolume{ socketPos, m_axisLength },
                    drawOptions,
                    m_axisMeshZ->getMaterial()->m_registeredIndex,
                    m_programId,
                    true);
            }
        }
    }

    drawObjects(ctx, targetBuffer, m_meshes);
}
