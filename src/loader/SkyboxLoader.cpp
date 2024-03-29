#include "SkyboxLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "asset/Assets.h"

#include "pool/NodeHandle.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/ModelMesh.h"

#include "event/Dispatcher.h"

#include "model/Node.h"

#include "registry/Registry.h"
#include "registry/ModelRegistry.h"
#include "registry/ProgramRegistry.h"
#include "registry/MeshTypeRegistry.h"

#include "scene/SkyboxMaterial.h"

namespace {
    const std::string SKYBOX_MESH_NAME{ "quad_skybox" };

    const std::vector<std::regex> hdriMatchers{
        std::regex(".*[\\.]hdr"),
    };
}

namespace loader {
    SkyboxLoader::SkyboxLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void SkyboxLoader::loadSkybox(
        const YAML::Node& node,
        SkyboxData& data)
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "program" || k == "shader") {
                data.programName = readString(v);
                data.programName = "skybox";
            }
            else if (k == "material") {
                data.materialName = readString(v);
            }
            else if (k == "priority") {
                data.priority = readInt(v);
            }
            else if (k == "gamma_correct") {
                data.gammaCorrect = readBool(v);
            }
            else if (k == "hdri") {
                data.hdri = readBool(v);
            }
            else if (k == "swap_faces") {
                data.swapFaces = readBool(v);
            }
            else if (k == "faces") {
                loadSkyboxFaces(v, data);
            }
            else {
                reportUnknown("skybox_entry", k, v);
            }
        }

        if (util::matchAny(hdriMatchers, data.materialName)) {
            data.hdri = true;
        }

        if (data.hdri) {
            data.gammaCorrect = false;
        }
    }

    void SkyboxLoader::loadSkyboxFaces(
        const YAML::Node& node,
        SkyboxData& data)
    {
        if (!node.IsSequence()) {
            return;
        }

        int idx = 0;
        for (const auto& e : node) {
            data.faces[idx] = e.as<std::string>();
            idx++;
        }

        data.loadedFaces = true;
    }

    void SkyboxLoader::attachSkybox(
        const ki::node_id rootId,
        const SkyboxData& data)
    {
        if (!data.valid()) return;

        const auto& assets = Assets::get();

        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();
        type->setName("<skybox>");

        type->m_priority = data.priority;

        auto future = ModelRegistry::get().getMesh(
            SKYBOX_MESH_NAME,
            assets.modelsDir);
        auto* mesh = future.get();
        type->addLod({ mesh });

        type->m_entityType = mesh::EntityType::skybox;

        auto& flags = type->m_flags;

        flags.skybox = true;
        flags.wireframe = false;
        flags.renderBack = true;
        flags.noShadow = true;
        flags.noFrustum = true;
        //flags.noReflect = true;
        //flags.noRefract = true;
        flags.noSelect = true;
        flags.noNormals = true;
        flags.gbuffer = false;// data.programName.starts_with("g_");

        type->m_program = ProgramRegistry::get().getProgram(data.programName);

        bool gammaCorrect = data.gammaCorrect;
        if (data.hdri) {
            gammaCorrect = false;
        }

        auto material{ std::make_unique<SkyboxMaterial>(
            data.materialName,
            gammaCorrect) };
        material->m_swapFaces = data.swapFaces;
        material->m_hdri = data.hdri;
        if (data.loadedFaces) {
            material->m_faces = data.faces;
        }

        MeshTypeRegistry::get().registerCustomMaterial(typeHandle);

        type->setCustomMaterial(std::move(material));

        auto handle = pool::NodeHandle::allocate(assets.skyboxId);
        auto* node = handle.toNode();
#ifdef _DEBUG
        node->m_resolvedSID = "<skybox>";
#endif
        node->m_typeHandle = typeHandle;

        {
            event::Event evt { event::Type::node_add };
            evt.body.node = {
                .target = assets.skyboxId,
                .parentId = rootId,
            };
            m_dispatcher->send(evt);
        }
    }

}
