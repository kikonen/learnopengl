#include "CubeMapLoader.h"

#include "util/Util.h"

#include "pool/NodeHandle.h"

#include "asset/Assets.h"

#include "material/Material.h"

#include "shader/Shader.h"
#include "shader/ProgramRegistry.h"

#include "event/Dispatcher.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/MeshSet.h"

#include "model/Node.h"

#include "registry/Registry.h"
#include "registry/ModelRegistry.h"

#include "loader/document.h"
#include "loader_util.h"

namespace loader {
    CubeMapLoader::CubeMapLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void CubeMapLoader::attachCubeMap(
        const ki::node_id rootId)
    {
        const auto& assets = Assets::get();

        if (!assets.showCubeMapCenter) return;

        auto future = ModelRegistry::get().getMeshSet(
            "",
            assets.modelsDir,
            "ball_volume");

        auto* meshSet = future.get();

        if (!meshSet) {
            KI_ERROR("Failed to load cubemap mesh");
            return;
        }

        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();
        type->setName("<cube_map>");

        type->addMeshSet(*meshSet);

        auto* lodMesh = type->modifyLodMesh (0);
        {
            auto material = Material::createMaterial(BasicMaterial::highlight);
            material.m_name = "cube_map";
            //material.kd = glm::vec4(0.f, 0.8f, 0.8f, 1.f);
            material.kd = glm::vec4(0.7516f, 0.6065f, 0.2265f, 1.f);

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

        auto handle = pool::NodeHandle::allocate(assets.cubeMapId);
        auto* node = handle.toNode();

        node->setName("<cube_map>");
        node->m_typeHandle = typeHandle;

        node->m_visible = false;

        {
            auto& state = node->modifyState();

            //node->setScale(m_asyncLoader->assets.cubeMapFarPlane);
            state.setScale(4.f);

            // NOTE KI m_radius = 1.73205078
            state.setVolume(meshSet->calculateAABB(glm::mat4{ 1.f }).getVolume());
        }

        {
            NodeState state{};
            event::Event evt { event::Type::node_add };
            evt.blob = std::make_unique<event::BlobData>();
            evt.blob->body.state = state;
            evt.body.node = {
                .target = assets.cubeMapId,
                .parentId = rootId,
            };
            assert(evt.body.node.target > 1);
            m_dispatcher->send(evt);
        }
    }
}
