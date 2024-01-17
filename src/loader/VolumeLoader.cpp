#include "VolumeLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "asset/Material.h"
#include "asset/Shader.h"

#include "mesh/MeshType.h"
#include "mesh/ModelMesh.h"

#include "controller/VolumeController.h"

#include "event/Dispatcher.h"

#include "model/Node.h"


#include "registry/Registry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/ProgramRegistry.h"

namespace {
}

namespace loader {
    VolumeLoader::VolumeLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void VolumeLoader::attachVolume(
        const ki::node_id rootId)
    {
        if (!m_assets.showVolume) return;

        auto type = m_registry->m_typeRegistry->registerType("<volume>");

        auto future = m_registry->m_modelRegistry->getMesh(
            "ball_volume",
            m_assets.modelsDir);
        auto* mesh = future.get();

        type->setMesh(mesh);

        {
            auto material = Material::createMaterial(BasicMaterial::highlight);
            material.m_name = "volume";
            material.kd = glm::vec4(0.8f, 0.8f, 0.f, 1.f);

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

        auto node = new Node(m_ctx.m_assets.volumeId);
        node->m_type = type;

        node->m_visible = false;

        // NOTE KI m_radius = 1.73205078
        mesh->prepareVolume();

        auto& transform = node->modifyTransform();

        transform.setVolume(mesh->getAABB().getVolume());

        {
            event::Event evt { event::Type::node_add };
            evt.body.node = {
                .target = node,
                .id = m_assets.volumeId,
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
