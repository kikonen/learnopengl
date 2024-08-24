#include "VolumeLoader.h"

#include <fmt/format.h>

#include "util/Util.h"
#include "util/Log.h"
#include "util/glm_format.h"

#include "asset/Assets.h"

#include "pool/NodeHandle.h"

#include "material/Material.h"

#include "shader/Shader.h"
#include "shader/ProgramRegistry.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"
#include "mesh/generator/PrimitiveGenerator.h"

#include "controller/VolumeController.h"

#include "event/Dispatcher.h"

#include "model/Node.h"

#include "mesh/ModelMesh.h"

#include "registry/Registry.h"
#include "registry/ModelRegistry.h"

#include "loader/document.h"
#include "loader_util.h"

namespace loader {
    VolumeLoader::VolumeLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void VolumeLoader::attachVolume(
        const ki::node_id rootId)
    {
        const auto& assets = Assets::get();

        if (!assets.showVolume) return;

        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();
        type->setName("<volume>");

        {
            auto generator = mesh::PrimitiveGenerator::sphere();
            generator.radius = 1.f;
            generator.slices = 8;
            generator.segments = { 4, 0, 0 };

            mesh::LodMesh lodMesh;
            lodMesh.setMesh(generator.create(), true);
            type->addLodMesh(std::move(lodMesh));
        }

        auto* lodMesh = type->modifyLodMesh(0);
        {
            auto material = Material::createMaterial(BasicMaterial::highlight);
            material.m_name = "volume";
            material.kd = glm::vec4(0.8f, 0.8f, 0.f, 1.f);

            material.renderBack = true;
            material.wireframe = true;
            material.gbuffer = SHADER_VOLUME.starts_with("g_");

            lodMesh->setMaterial(material);
            lodMesh->m_program = ProgramRegistry::get().getProgram(SHADER_VOLUME);
        }

        type->m_nodeType = NodeType::marker;

        auto& flags = type->m_flags;

        flags.noShadow = true;
        flags.noFrustum = false;
        flags.noReflect = true;
        flags.noRefract = true;
        flags.noSelect = true;
        flags.noNormals = true;

        auto handle = pool::NodeHandle::allocate(assets.volumeId);
        auto* node = handle.toNode();

        node->setName("<volume>");
        node->m_typeHandle = typeHandle;

        node->m_visible = false;

        // NOTE KI m_radius = 1.73205078
        {
            auto& state = node->modifyState();

            state.setBaseRotation(util::degreesToQuat(glm::vec3{ 90.f, 0, 0 }));
            state.setVolume(lodMesh->calculateAABB().getVolume());
        }

        {
            event::Event evt { event::Type::node_add };
            evt.body.node = {
                .target = assets.volumeId,
                .parentId = rootId,
            };
            m_dispatcher->send(evt);
        }
        {
            auto* controller = new VolumeController();

            event::Event evt { event::Type::controller_add };
            evt.body.control = {
                .target = node->getId(),
                .controller = controller
            };
            m_dispatcher->send(evt);
        }
    }
}
