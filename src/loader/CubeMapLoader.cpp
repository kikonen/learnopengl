#include "CubeMapLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "pool/NodeHandle.h"

#include "asset/Assets.h"
#include "asset/Material.h"
#include "asset/Shader.h"

#include "event/Dispatcher.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/ModelMesh.h"

#include "model/Node.h"

#include "registry/Registry.h"
#include "registry/ModelRegistry.h"
#include "registry/ProgramRegistry.h"

namespace {
}

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

        auto future = ModelRegistry::get().getMesh(
            assets.modelsDir,
            "ball_volume");

        auto* mesh = future.get();

        if (!mesh) {
            KI_ERROR("Failed to load cubemap mesh");
            return;
        }

        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();
        type->setName("<cube_map>");

        auto* lod = type->addLod({ mesh });

        type->m_entityType = mesh::EntityType::marker;

        {
            auto material = Material::createMaterial(BasicMaterial::highlight);
            material.m_name = "cube_map";
            //material.kd = glm::vec4(0.f, 0.8f, 0.8f, 1.f);
            material.kd = glm::vec4(0.7516f, 0.6065f, 0.2265f, 1.f);

            lod->m_materialSet.setMaterials({ material });
        }

        auto& flags = type->m_flags;

        flags.wireframe = true;
        flags.renderBack = true;
        flags.noShadow = true;
        flags.noFrustum = false;
        flags.noReflect = true;
        flags.noRefract = true;
        flags.noSelect = true;
        flags.noNormals = true;
        flags.gbuffer = SHADER_VOLUME.starts_with("g_");

        type->m_program = ProgramRegistry::get().getProgram(SHADER_VOLUME);

        auto handle = pool::NodeHandle::allocate(assets.cubeMapId);
        auto* node = handle.toNode();
#ifdef _DEBUG
        node->m_resolvedSID = "<cube_map>";
#endif
        node->m_typeHandle = typeHandle;

        node->m_visible = false;

        auto& transform = node->modifyTransform();

        //node->setScale(m_asyncLoader->assets.cubeMapFarPlane);
        transform.setScale(4.f);

        // NOTE KI m_radius = 1.73205078
        mesh->prepareVolume();
        transform.setVolume(mesh->getAABB().getVolume());

        {
            event::Event evt { event::Type::node_add };
            evt.body.node = {
                .target = assets.cubeMapId,
                .parentId = rootId,
            };
            m_dispatcher->send(evt);
        }
    }
}
