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
                    data.type = EntityType::origo;
                }
                else if (type == "container") {
                    data.type = EntityType::container;
                }
                else if (type == "model") {
                    data.type = EntityType::model;
                }
                else if (type == "text") {
                    data.type = EntityType::text;
                }
                else if (type == "terrain") {
                    data.type = EntityType::terrain;
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
            else if (k == "model") {
                if (data.type == EntityType::none) {
                    auto& meshData = data.meshes.emplace_back();
                    loadMesh(v, meshData, loaders);
                }
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
                data.programs[MaterialProgramType::geometry] = readString(v);
            }
            else if (k == "program_definitions") {
                for (const auto& defNode : v.getNodes()) {
                    const auto& defName = defNode.getName();
                    const auto& defValue = readString(defNode.getNode());
                    data.programDefinitions[util::toUpper(defName)] = defValue;
                }
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
                loadText(v, data.text);
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
            else if (k == "models" || k == "meshes") {
                loadMeshes(v, data.meshes, loaders);
            }
            else if (k == "lods") {
                loadLods(v, data.lods);
            }
            else {
                reportUnknown("entity_entry", k, v);
            }
        }

        if (data.type == EntityType::none) {
            if (!data.meshes.empty()) {
                data.type = EntityType::model;
            }
        }

        if (data.enabled) {
            if (!data.meshes.empty()) {
                if (data.type != EntityType::model) {
                    auto msg = fmt::format("INVALID: type is not model - id={}, name={}", data.baseId, data.name);
                    KI_INFO_OUT(msg);
                    throw msg;
                }
            }

            if (data.type == EntityType::none) {
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

    void EntityLoader::loadText(
        const loader::DocNode& node,
        TextData& data) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "text") {
                data.text = readString(v);
            }
            else if (k == "font") {
                data.font = readString(v);
            }
            else {
                reportUnknown("text_entry", k, v);
            }
        }
    }

    void EntityLoader::loadMeshes(
        const loader::DocNode& node,
        std::vector<MeshData>& meshes,
        Loaders& loaders) const
    {
        int level = 0;
        for (const auto& entry : node.getNodes()) {
            auto& data = meshes.emplace_back();
            data.level = level;
            loadMesh(entry, data, loaders);
            level++;
        }
    }

    void EntityLoader::loadMesh(
        const loader::DocNode& node,
        MeshData& data,
        Loaders& loaders) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::DocNode& v = pair.getNode();

            const auto k = util::toLower(key);

            if (k == "level") {
                data.level = readInt(v);
            }
            else if (k == "mesh") {
                if (v.isSequence()) {
                    auto& nodes = v.getNodes();
                    data.meshPath = util::joinPath(readString(nodes[0]), readString(nodes[1]));
                }
                else {
                    data.meshPath = readString(v);
                }

                if (data.baseDir.empty()) {
                    std::filesystem::path path{ data.meshPath };
                    data.baseDir = path.parent_path().string();
                }
            }
            else if (k == "base_dir") {
                data.baseDir = readString(v);
            }
            else if (k == "animations") {
                loadAnimations(v, data.animations);
            }
            else if (k == "materials") {
                loaders.m_materialLoader.loadMaterials(v, data.materials);
            }
            else if (k == "material") {
                if (data.materials.empty()) {
                    data.materials.emplace_back();
                }
                auto& materialData = data.materials[0];
                materialData.aliasName = "*";
                loaders.m_materialLoader.loadMaterial(v, materialData);
                materialData.materialName = materialData.material.m_name;
            }
            else if (k == "material_modifier") {
                if (data.materials.empty()) {
                    data.materials.emplace_back();
                }
                auto& materialData = data.materials[0];
                materialData.aliasName = "*";
                materialData.modify = true;
                loaders.m_materialLoader.loadMaterialModifiers(v, materialData);
            } else {
                reportUnknown("model_entry", k, v);
            }
        }

        for (auto& materialData : data.materials) {
            loaders.m_materialLoader.resolveMaterialPaths(data.baseDir, materialData);
            loaders.m_materialLoader.resolveMaterialPbr(data.baseDir, materialData);
        }
    }

    void EntityLoader::loadLods(
        const loader::DocNode& node,
        std::vector<LodData>& lods) const
    {
        int level = 0;
        for (const auto& entry : node.getNodes()) {
            LodData& data = lods.emplace_back();
            data.level = level;
            loadLod(entry, data);
            level++;
        }
    }

    void EntityLoader::loadLod(
        const loader::DocNode& node,
        LodData& data) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::DocNode& v = pair.getNode();

            const auto k = util::toLower(key);

            if (k == "level") {
                data.level = readInt(v);
            }
            else if (k == "distance") {
                data.distance = readFloat(v);
            }
            else {
                reportUnknown("lod_entry", k, v);
            }
        }
    }

    void EntityLoader::loadAnimations(
        const loader::DocNode& node,
        std::vector<AnimationData>& animations) const
    {
        for (const auto& entry : node.getNodes()) {
            AnimationData& data = animations.emplace_back();
            loadAnimation(entry, data);
        }
    }

    void EntityLoader::loadAnimation(
        const loader::DocNode& node,
        AnimationData& data) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::DocNode& v = pair.getNode();

            const auto k = util::toLower(key);

            if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "path") {
                data.path = readString(v);
            }
            else {
                reportUnknown("animation_entry", k, v);
            }
        }
    }
}
