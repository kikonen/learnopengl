#include "CubeMapLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "pool/NodeHandle.h"

#include "asset/Material.h"
#include "asset/Shader.h"

#include "event/Dispatcher.h"

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
        if (!m_assets.showCubeMapCenter) return;

        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();
        type->setName("<cube_map>");

        auto future = m_registry->m_modelRegistry->getMesh(
            "ball_volume",
            m_assets.modelsDir);
        auto& mesh = future.get();

        type->setMesh(mesh);

        {
            auto material = Material::createMaterial(BasicMaterial::highlight);
            material.m_name = "cube_map";
            //material.kd = glm::vec4(0.f, 0.8f, 0.8f, 1.f);
            material.kd = glm::vec4(0.7516f, 0.6065f, 0.2265f, 1.f);

            auto& materialVBO = type->m_materialVBO;
            materialVBO->setDefaultMaterial(material, true, true);
            materialVBO->setMaterials({ material });
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

        type->m_program = m_registry->m_programRegistry->getProgram(SHADER_VOLUME);

        auto handle = pool::NodeHandle::allocate(m_ctx.m_assets.cubeMapId);
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
                .target = m_assets.cubeMapId,
                .parentId = rootId,
            };
            m_dispatcher->send(evt);
        }
    }
}
