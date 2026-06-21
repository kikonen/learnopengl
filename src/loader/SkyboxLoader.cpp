#include "SkyboxLoader.h"

#include "util/util.h"
#include "util/Ref.h"

#include "ki/sid.h"

#include "asset/Assets.h"

#include "pool/NodeHandle.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshSet.h"

#include "event/Dispatcher.h"

#include "engine/Engine.h"
#include "engine/PrepareContext.h"

#include "model/Node.h"
#include "model/NodeType.h"
#include "model/CreateState.h"

#include "shader/ProgramRegistry.h"

#include "registry/Registry.h"
#include "registry/NodeTypeRegistry.h"

#include "scene/Scene.h"
#include "scene/Skybox.h"
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
        const std::shared_ptr<Context>& ctx)
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
            else if (k == "material_2") {
                data.materialName2 = readString(v);
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
        const SkyboxData& data)
    {
        if (!data.valid()) return;

        const auto& assets = Assets::get();

        bool gammaCorrect = data.gammaCorrect;
        if (data.hdri) {
            gammaCorrect = false;
        }

        auto skybox = util::Ref<Skybox>::create();
        {
            auto material = util::Ref<SkyboxMaterial>::create(
                UNIT_SKYBOX_DAY,
                data.materialName,
                gammaCorrect);
            material->m_hdri = data.hdri;
            material->m_swapFaces = data.swapFaces;
            if (data.loadedFaces) {
                material->m_faces = data.faces;
            }
            skybox->setMaterial(material, 0);
        }
        if (!data.materialName2.empty()) {
            auto material = util::Ref<SkyboxMaterial>::create(
                UNIT_SKYBOX_NIGHT,
                data.materialName2,
                gammaCorrect);
            material->m_hdri = data.hdri;
            material->m_swapFaces = data.swapFaces;
            if (data.loadedFaces2) {
                material->m_faces = data.faces2;
            }
            skybox->setMaterial(material, 1);
        }

        {
            auto fn = [
                registry = m_registry.get(),
                skybox = skybox]() {
                auto scene = registry->getEngine().getCurrentScene();
                scene->setSkybox(skybox);

                PrepareContext ctx{ registry->getEngine() };
                skybox->prepareRT(ctx);
            };
            m_registry->invokeLaterRT(std::move(fn));
        }
    }

}
