#include "VolumeLoader.h"

#include "util/Util.h"

#include "asset/Assets.h"

#include "pool/NodeHandle.h"

#include "asset/Material.h"
#include "asset/Shader.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/MeshSet.h"

#include "controller/VolumeController.h"

#include "event/Dispatcher.h"

#include "model/Node.h"


#include "registry/Registry.h"
#include "registry/ModelRegistry.h"
#include "registry/ProgramRegistry.h"

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

        auto future = ModelRegistry::get().getMeshSet(
            assets.modelsDir,
            "ball_volume");
        auto* meshSet = future.get();

        if (!meshSet) {
            KI_ERROR("Failed to load volume mesh");
            return;
        }

        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();
        type->setName("<volume>");

        type->addMeshSet(*meshSet);

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

            state.setVolume(meshSet->calculateAABB(glm::mat4{ 1.f }).getVolume());
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
