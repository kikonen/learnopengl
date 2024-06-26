#include "EntityLoader.h"

#include <string>
#include <vector>
#include <algorithm>
#include <string>

#include <fmt/format.h>

#include "util/Util.h"
#include "util/glm_format.h"

#include "asset/Shader.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "registry/Registry.h"
#include "registry/ModelRegistry.h"

#include "Loaders.h"

#include "loader/document.h"

namespace loader {
    EntityLoader::EntityLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void EntityLoader::loadEntities(
        const loader::DocNode& node,
        std::vector<EntityRoot>& entities,
        Loaders& loaders) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& entityRoot = entities.emplace_back();
            loadEntity(
                entry,
                entityRoot,
                loaders);
        }
    }

    void EntityLoader::loadEntity(
        const loader::DocNode& node,
        EntityRoot& entityRoot,
        Loaders& loaders) const
    {
        loadEntityClone(
            node,
            entityRoot.base,
            entityRoot.clones,
            true,
            loaders);
    }

    void EntityLoader::loadEntityClone(
        const loader::DocNode& node,
        EntityData& data,
        std::vector<EntityData>& clones,
        bool recurse,
        Loaders& loaders) const
    {
        bool hasClones = false;

        data.enabled = true;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "type") {
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
                    reportUnknown("entity_type", k, v);
                }
            }
            else if (k == "xid") {
                data.baseId = readId(v);
                data.enabled = false;
            }
            else if (k == "id") {
                data.baseId = readId(v);
            }
            else if (k == "parent_id") {
                data.parentBaseId = readId(v);
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
            else if (k == "priority") {
                data.priority = readInt(v);
            }
            else if (k == "mesh" || k == "model") {
                auto& meshData = data.meshes.emplace_back();
                loaders.m_meshLoader.loadMesh(v, meshData, loaders);
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
            else if (k == "geometry_type") {
                data.geometryType = readString(v);
            }
            else if (k == "type_flags" || k == "render_flags") {
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
            else if (k == "base_scale") {
                data.baseScale = readScale3(v);
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
                loaders.m_customMaterialLoader.loadCustomMaterial(v, data.customMaterial);
            }
            else if (k == "physics") {
                loaders.m_physicsLoader.loadPhysics(v, data.physics);
            }
            else if (k == "controllers") {
                loaders.m_controllerLoader.loadControllers(v, data.controllers);
            }
            else if (k == "controller") {
                auto& controllerDaata = data.controllers.emplace_back();
                loaders.m_controllerLoader.loadController(v, controllerDaata);
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
                loaders.m_scriptLoader.loadScript(v, data.script);
            }
            else if (k == "script_file") {
                loaders.m_scriptLoader.loadScript(v, data.script);
            }
            else if (k == "meshes") {
                loaders.m_meshLoader.loadMeshes(v, data.meshes, loaders);
            }
            else {
                reportUnknown("entity_entry", k, v);
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
                throw msg;
            }
        }

        if (hasClones) {
            for (const auto& pair : node.getNodes()) {
                const std::string& k = pair.getName();
                const loader::DocNode& v = pair.getNode();

                if (k == "clones") {
                    for (const auto& node : v.getNodes()) {
                        // NOTE KI intialize with current data
                        EntityData clone = data;
                        std::vector<EntityData> dummy{};
                        loadEntityClone(
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

}
