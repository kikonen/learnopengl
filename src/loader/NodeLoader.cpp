#include "NodeLoader.h"

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

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "registry/Registry.h"
#include "registry/ModelRegistry.h"

#include "Loaders.h"

#include "loader/converter/YamlConverter.h"
#include "loader/document.h"
#include "loader_util.h"

#include "PivotLoader.h"

namespace {
    int readLayer(const loader::DocNode& node)
    {
        const auto& name = util::toLower(readString(node));
        const auto* layer = LayerInfo::findLayer(name);
        return layer ? layer->m_index : LAYER_NONE_INDEX;
    }
}

namespace loader {
    NodeLoader::NodeLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void NodeLoader::loadNodes(
        const loader::DocNode& node,
        std::vector<NodeRoot>& nodes,
        Loaders& loaders) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& nodeRoot = nodes.emplace_back();
            loadNode(
                entry,
                nodeRoot,
                loaders);
        }
    }

    void NodeLoader::loadNode(
        const loader::DocNode& node,
        NodeRoot& nodeRoot,
        Loaders& loaders) const
    {
        loadNodeClone(
            node,
            nodeRoot.base,
            nodeRoot.clones,
            true,
            loaders);
    }

    void NodeLoader::loadNodeClone(
        const loader::DocNode& node,
        NodeData& data,
        std::vector<NodeData>& clones,
        bool recurse,
        Loaders& loaders) const
    {
        bool hasClones = false;

        data.enabled = true;

        if (const auto* layer = LayerInfo::findLayer(LAYER_MAIN); layer) {
            data.layer = layer->m_index;
        }

        if (recurse) {
            loadPrefab(node.findNode("prefab"), data, loaders);
        }

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "prefab") {
                // NOTE KI loaded as "pre step"
            }
            else if (k == "type") {
                std::string type = readString(v);
                if (type == "origo") {
                    data.type = NodeType::origo;
                }
                else if (type == "container") {
                    data.type = NodeType::container;
                }
                else if (type == "model") {
                    data.type = NodeType::model;
                }
                else if (type == "text") {
                    data.type = NodeType::text;
                }
                else if (type == "terrain") {
                    data.type = NodeType::terrain;
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
            else if (k == "parent_id" || k == "parent") {
                data.parentBaseId = readId(v);
            }
            else if (k == "ignored_by") {
                data.ignoredByBaseId = readId(v);
            }
            else if (k == "xxname" || k == "xname") {
				// NOTE quick disable logic
                data.name = readString(v);
                data.enabled = false;
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
            else if (k == "active") {
                data.active = readBool(v);
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
            else if (k == "node_flags") {
                for (const auto& flagNode : v.getNodes()) {
                    const auto& flagName = flagNode.getName();
                    const auto& flagValue = readBool(flagNode.getNode());
                    data.nodeFlags.set(util::toLower(flagName), flagValue);
                }
            }
            else if (k == "front") {
                data.front = readVec3(v);
            }
            else if (k == "text") {
                loaders.m_textLoader.loadText(v, data.text, loaders);
            }
            else if (k == "position" || k == "pos") {
                data.position = readVec3(v);
            }
            else if (k == "base_rot" || k == "base_rotation") {
                data.baseRotation = readDegreesRotation(v);
            }
            else if (k == "rot" || k == "rotation") {
                data.rotation = readDegreesRotation(v);
            }
            else if (k == "scale") {
                data.scale = readScale3(v);
            }
            else if (k == "mesh_scale" || k == "base_scale") {
                data.meshScale = readScale3(v);
            }
            else if (k == "pivot") {
                data.pivot = PivotLoader{}.load(v);
            }
            else if (k == "repeat") {
                loadRepeat(v, data.repeat);
            }
            else if (k == "tiling") {
                loadTiling(v, data.tiling);
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
            else if (k == "selected") {
                data.selected = readBool(v);
            }
            else if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "xxenabled" || k == "xenabled") {
                // NOTE compat with old "disable" logic
                data.enabled = false;
            }
            else if (k == "clone_position_offset") {
                data.clonePositionOffset = readVec3(v);
            }
            else if (k == "clone_mesh") {
                data.cloneMesh = readBool(v);
            }
            else if (k == "tile") {
                data.tile = readVec3(v);
            }
            else if (k == "clones") {
                if (recurse)
                    hasClones = true;
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

        if (data.type == NodeType::none) {
            if (!data.meshes.empty()) {
                data.type = NodeType::model;
            }
            if (data.text.enabled) {
                data.type = NodeType::text;
            }
        }

        if (data.enabled) {
            //if (!data.meshes.empty()) {
            //    if (data.type != NodeType::model) {
            //        auto msg = fmt::format("INVALID: type is not model - id={}, name={}", data.baseId, data.name);
            //        KI_INFO_OUT(msg);
            //        throw msg;
            //    }
            //}

            if (data.type == NodeType::none) {
                auto msg = fmt::format("INVALID: type missing - id={}, name={}", data.baseId, data.name);
                KI_INFO_OUT(msg);
                //throw msg;
                data.enabled = false;
            }
        }

        if (data.type == NodeType::text) {
            for (auto& meshData : data.meshes) {
                meshData.enabled = true;
            }
        }

        for (auto& meshData : data.meshes) {
            if (!meshData.hasScale) {
                meshData.scale = data.meshScale;
            }
        }

        if (hasClones) {
            for (const auto& pair : node.getNodes()) {
                const std::string& k = pair.getName();
                const loader::DocNode& v = pair.getNode();

                if (k == "clones") {
                    for (const auto& node : v.getNodes()) {
                        // NOTE KI intialize with current data
                        NodeData clone = data;
                        std::vector<NodeData> dummy{};
                        loadNodeClone(
                            node,
                            clone,
                            dummy,
                            false,
                            loaders);
                        clones.push_back(clone);
                    }
                }
            }
        }
    }

    void NodeLoader::loadPrefab(
        const loader::DocNode& node,
        NodeData& data,
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

        const auto& fullPath = util::joinPath(m_ctx.m_dirName, path);

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
                loadNodeClone(
                    v,
                    data,
                    clones,
                    false,
                    loaders);
            }
        }
    }

    void NodeLoader::loadAttachments(
        const loader::DocNode& node,
        std::vector<AttachmentData>& attachments) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& data = attachments.emplace_back();
            loadAttachment(entry, data);
        }
    }

    void NodeLoader::loadAttachment(
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
