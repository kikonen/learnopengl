#include "CubeMapLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "asset/Material.h"
#include "asset/Shader.h"
#include "asset/ModelMesh.h"

#include "event/Dispatcher.h"

#include "model/Node.h"

#include "registry/Registry.h"
#include "registry/MeshType.h"
#include "registry/MeshTypeRegistry.h"
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
        const uuids::uuid& rootId)
    {
        if (!m_assets.showCubeMapCenter) return;

        auto* type = m_registry->m_typeRegistry->registerType("<cube_map>");

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
            materialVBO.setDefaultMaterial(material, true, true);
            materialVBO.setMaterials({ material });
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

        auto node = new Node(type);
        node->m_visible = false;

        //node->setScale(m_asyncLoader->assets.cubeMapFarPlane);
        node->getTransform().setScale(4.f);

        // NOTE KI m_radius = 1.73205078
        mesh->prepareVolume();

        node->setVolume(mesh->getAABB().getVolume());

        {
            event::Event evt { event::Type::node_add };
            evt.body.node = {
                .target = node,
                .uuid = m_assets.cubeMapUUID,
                .parentUUID = rootId,
            };
            m_dispatcher->send(evt);
        }
    }
}
