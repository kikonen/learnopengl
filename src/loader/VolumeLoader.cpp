#include "VolumeLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "asset/Assets.h"

#include "pool/NodeHandle.h"

#include "asset/Material.h"
#include "asset/Shader.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/ModelMesh.h"

#include "controller/VolumeController.h"

#include "event/Dispatcher.h"

#include "model/Node.h"


#include "registry/Registry.h"
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
        const auto& assets = Assets::get();

        if (!assets.showVolume) return;

        auto future = ModelRegistry::get().getMesh(
            "ball_volume",
            assets.modelsDir);
        auto* mesh = future.get();

        if (!mesh) {
            KI_ERROR("Failed to load volume mesh");
            return;
        }

        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();
        type->setName("<volume>");

        auto* lod = type->addLod({ mesh });

        type->m_entityType = mesh::EntityType::marker;

        {
            auto material = Material::createMaterial(BasicMaterial::highlight);
            material.m_name = "volume";
            material.kd = glm::vec4(0.8f, 0.8f, 0.f, 1.f);

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

        auto handle = pool::NodeHandle::allocate(assets.volumeId);
        auto* node = handle.toNode();
#ifdef _DEBUG
        node->m_resolvedSID = "<volume>";
#endif
        node->m_typeHandle = typeHandle;

        node->m_visible = false;

        // NOTE KI m_radius = 1.73205078
        mesh->prepareVolume();

        auto& transform = node->modifyTransform();

        transform.setVolume(mesh->getAABB().getVolume());

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
