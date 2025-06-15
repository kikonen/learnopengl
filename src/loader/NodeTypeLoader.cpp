#include "NodeTypeLoader.h"

#include <string>
#include <vector>
#include <algorithm>
#include <string>

#include <fmt/format.h>

#include "util/util.h"
#include "util/file.h"
#include "util/glm_format.h"

#include "asset/Assets.h"

#include "shader/Shader.h"

#include "registry/Registry.h"

#include "Context.h"
#include "Loaders.h"

#include "loader/converter/YamlConverter.h"
#include "loader/document.h"
#include "loader_util.h"

#include "PivotLoader.h"

#include "NodeTypeData.h"

namespace {
}

namespace loader {
    NodeTypeLoader::NodeTypeLoader(
        std::shared_ptr<Context> ctx)
        : BaseLoader(ctx)
    {
    }

    void NodeTypeLoader::loadNodeTypes(
        const loader::DocNode& node,
        std::vector<NodeTypeData>& nodeTypes,
        Loaders& loaders) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& data = nodeTypes.emplace_back();
            loadNodeType(
                entry,
                data,
                loaders);
        }
    }

    void NodeTypeLoader::loadNodeType(
        const loader::DocNode& node,
        NodeTypeData& data,
        Loaders& loaders) const
    {
        bool hasClones = false;

        data.enabled = true;

        if (const auto* layer = LayerInfo::findLayer(LAYER_MAIN); layer) {
            data.layer = layer->m_index;
        }

        loadPrefab(node.findNode("prefab"), data, loaders);

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "prefab") {
                // NOTE KI loaded as "pre step"
            }
            else if (k == "type") {
                std::string type = readString(v);
                if (type == "origo") {
                    data.type = NodeKind::origo;
                }
                else if (type == "container") {
                    data.type = NodeKind::container;
                }
                else if (type == "model") {
                    data.type = NodeKind::model;
                }
                else if (type == "text") {
                    data.type = NodeKind::text;
                }
                else if (type == "terrain") {
                    data.type = NodeKind::terrain;
                }
                else {
                    reportUnknown("node_type", k, v);
                }
            }
            else if (k == "xid") {
                data.baseId = readId(v);
                data.enabled = false;
            }
            else if (k == "id") {
                data.baseId = readId(v);
            }
            else if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "desc") {
                data.desc = readString(v);
            }
            else if (k == "prefab") {
                data.prefabName = readString(v);
            }
            else if (k == "layer") {
                data.layer = readLayer(v);
            }
            else if (k == "priority") {
                data.priority = readInt(v);
            }
            else if (k == "mesh" || k == "model") {
                if (data.meshes.empty()) {
                    data.meshes.emplace_back();
                }
                loaders.m_meshLoader.loadMesh(v, data.meshes[0], loaders);
            }
            else if (k == "program") {
                data.programs[MaterialProgramType::shader] = readString(v);
            }
            else if (k == "shadow_program") {
                data.programs[MaterialProgramType::shadow] = readString(v);
            }
            else if (k == "pre_depth_program") {
                data.programs[MaterialProgramType::pre_depth] = readString(v);
            }
            else if (k == "selection_program") {
                data.programs[MaterialProgramType::selection] = readString(v);
            }
            else if (k == "id_program") {
                data.programs[MaterialProgramType::object_id] = readString(v);
            }
            else if (k == "flags" || k == "type_flags" || k == "render_flags") {
                for (const auto& flagNode : v.getNodes()) {
                    const auto& flagName = flagNode.getName();
                    const auto& flagValue = readBool(flagNode.getNode());
                    data.typeFlags.set(util::toLower(flagName), flagValue);
                }
            }
            else if (k == "front") {
                data.front = readVec3(v);
            }
            else if (k == "text") {
                loaders.m_textLoader.loadText(v, data.text, loaders);
            }
            else if (k == "base_rot" || k == "base_rotation") {
                data.baseRotation = readDegreesRotation(v);
            }
            else if (k == "base_scale") {
                data.baseScale = readScale3(v);
            }
            else if (k == "base_scale") {
                throw "use base_scale in mesh";
            }
            else if (k == "mesh_scale") {
                throw "use base_scale in mesh";
            }
            else if (k == "pivot") {
                data.pivot = PivotLoader{}.load(v);
            }
            else if (k == "camera") {
                loaders.m_cameraLoader.loadCamera(v, data.camera);
            }
            else if (k == "light") {
                loaders.m_lightLoader.loadLight(v, data.light);
            }
            else if (k == "audio") {
                loaders.m_audioLoader.loadAudio(v, data.audio);
            }
            else if (k == "custom_material") {
                loaders.m_customMaterialLoader.loadCustomMaterial(v, data.customMaterial, loaders);
            }
            else if (k == "physics") {
                loaders.m_physicsLoader.loadPhysics(v, data.physics);
            }
            else if (k == "controllers") {
                loaders.m_controllerLoader.loadControllers(v, data.controllers);
            }
            else if (k == "controller") {
                if (data.controllers.empty()) {
                    data.controllers.emplace_back();
                }
                loaders.m_controllerLoader.loadController(v, data.controllers[0]);
            }
            else if (k == "generator") {
                loaders.m_generatorLoader.loadGenerator(v, data.generator, loaders);
            }
            else if (k == "particle") {
                loaders.m_particleLoader.loadParticle(v, data.particle, loaders);
            }
            else if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "xxenabled" || k == "xenabled") {
                // NOTE compat with old "disable" logic
                data.enabled = false;
            }
            else if (k == "script") {
                auto& scriptData = data.scripts.emplace_back();
                loaders.m_scriptLoader.loadScript(v, scriptData, false);
            }
            else if (k == "scripts") {
                loaders.m_scriptLoader.loadScripts(v, data.scripts, false);
            }
            else if (k == "script_file") {
                auto& scriptData = data.scripts.emplace_back();
                loaders.m_scriptLoader.loadScript(v, scriptData, true);
            }
            else if (k == "script_files") {
                loaders.m_scriptLoader.loadScripts(v, data.scripts, true);
            }
            else if (k == "meshes") {
                loaders.m_meshLoader.loadMeshes(v, data.meshes, loaders);
            }
            else if (k == "attachments") {
                loadAttachments(v, data.attachments);
            }
            else {
                reportUnknown("node_entry", k, v);
            }
        }

        if (data.type == NodeKind::none) {
            if (!data.meshes.empty()) {
                data.type = NodeKind::model;
            }
            if (data.text.enabled) {
                data.type = NodeKind::text;
            }
        }

        if (data.enabled) {
            if (data.type == NodeKind::none) {
                auto msg = fmt::format("INVALID: type missing - id={}", data.baseId);
                KI_INFO_OUT(msg);
                data.enabled = false;
            }
        }

        if (data.type == NodeKind::text) {
            for (auto& meshData : data.meshes) {
                meshData.enabled = true;
            }
        }
    }

    void NodeTypeLoader::loadPrefab(
        const loader::DocNode& node,
        NodeTypeData& data,
        Loaders& loaders) const
    {
        if (node.isNull()) return;

        std::string path;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "path") {
                path = readString(v);
                break;
            }
        }

        if (path.empty()) {
            path = readString(node);
        }

        if (path.empty()) return;

        {
            std::filesystem::path filePath{ path };
            if (filePath.extension().empty()) {
                path += ".yml";
            }
        }

        const auto& fullPath = util::joinPath(m_ctx->m_dirName, path);

        KI_INFO_OUT(fmt::format("node_prefab={}", fullPath));

        if (!util::fileExists(fullPath))
        {
            throw fmt::format("INVALID: node_prefab missing - path={}", fullPath);
        }

        loader::YamlConverter converter;
        auto doc = converter.load(fullPath);

        for (const auto& pair : doc.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "prefab") {
                std::vector<NodeData> clones;
               loadNodeType(
                    v,
                    data,
                    loaders);
            }
        }
    }

    void NodeTypeLoader::loadAttachments(
        const loader::DocNode& node,
        std::vector<AttachmentData>& attachments) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& data = attachments.emplace_back();
            loadAttachment(entry, data);
        }
    }

    void NodeTypeLoader::loadAttachment(
        const loader::DocNode& node,
        AttachmentData& data) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::DocNode& v = pair.getNode();

            const auto k = util::toLower(key);

            if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "name" || k == "xname") {
                data.name = readString(v);
                data.enabled = k != "xname";
            }
            else if (k == "socket") {
                data.socket = readString(v);
            }
            else {
                reportUnknown("attachment_entry", k, v);
            }
        }
    }
}
