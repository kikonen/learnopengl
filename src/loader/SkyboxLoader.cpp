#include "SkyboxLoader.h"

#include "util/util.h"

#include "asset/Assets.h"

#include "pool/NodeHandle.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshSet.h"

#include "event/Dispatcher.h"

#include "model/Node.h"
#include "model/NodeType.h"

#include "shader/ProgramRegistry.h"

#include "registry/Registry.h"
#include "registry/ModelRegistry.h"
#include "registry/NodeTypeRegistry.h"

#include "scene/SkyboxMaterial.h"

#include "loader/document.h"
#include "loader_util.h"

namespace {
    const std::string SKYBOX_MESH_NAME{ "quad_skybox" };

    const std::vector<std::regex> hdriMatchers{
        std::regex(".*[\\.]hdr"),
    };
}

namespace loader {
    SkyboxLoader::SkyboxLoader(
        std::shared_ptr<Context> ctx)
        : BaseLoader(ctx)
    {
    }

    void SkyboxLoader::loadSkybox(
        const loader::DocNode& node,
        SkyboxData& data)
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

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
        const loader::DocNode& node,
        SkyboxData& data)
    {
        if (!node.isSequence()) {
            return;
        }

        int idx = 0;
        for (const auto& e : node.getNodes()) {
            data.faces[idx] = readString(e);
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

        // TODO KI just generate primitive mesh
        auto future = ModelRegistry::get().getMeshSet(
            "",
            assets.modelsDir,
            SKYBOX_MESH_NAME,
            false,
            false);
        const auto& meshSet = future.get();

        if (!meshSet) {
            KI_ERROR("Failed to load skybox mesh");
            return;
        }

        std::string name = "<skybox>";
        auto typeHandle = pool::TypeHandle::allocate(SID(name));
        auto* type = typeHandle.toType();
        type->setName(name);

        type->addMeshSet(*meshSet);

        if (const auto* layer = LayerInfo::findLayer(LAYER_MAIN); layer) {
            type->m_layer = layer->m_index;
        }

        auto* lodMesh = type->modifyLodMesh(0);
        {
            lodMesh->m_priority = data.priority;
            lodMesh->m_drawOptions.m_lineMode = false;
            lodMesh->m_drawOptions.m_renderBack = true;
            //lodMesh->flags.gbuffer = false;// data.programName.starts_with("g_");

            lodMesh->m_programId = ProgramRegistry::get().getProgram(data.programName);
        }

        auto& flags = type->m_flags;

        flags.skybox = true;
        flags.noShadow = true;
        flags.noFrustum = true;
        flags.noSelect = true;
        flags.noNormals = true;

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

        type->setCustomMaterial(std::move(material));

        auto handle = pool::NodeHandle::allocate(assets.skyboxId);
        auto* node = handle.toNode();

        node->setName("<skybox>");
        node->m_typeHandle = typeHandle;
        node->m_typeFlags = type->m_flags;
        node->m_layer = type->m_layer;

        //util::sleep(1000);

        {
            NodeState state{};
            event::Event evt { event::Type::node_add };
            evt.blob = std::make_unique<event::BlobData>();
            evt.blob->body.state = state;
            evt.body.node = {
                .target = assets.skyboxId,
                .parentId = rootId,
            };
            assert(evt.body.node.target > 1);
            m_dispatcher->send(evt);
        }
    }

}
