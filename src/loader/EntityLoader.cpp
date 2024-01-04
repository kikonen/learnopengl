#include "EntityLoader.h"

#include <string>
#include <vector>
#include <algorithm>
#include <string>

#include <fmt/format.h>

#include "util/Util.h"
#include "util/glm_format.h"

#include "ki/yaml.h"

#include "asset/Shader.h"

#include "mesh/MeshType.h"

#include "registry/Registry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/ModelRegistry.h"

#include "MaterialLoader.h"
#include "CustomMaterialLoader.h"
#include "SpriteLoader.h"
#include "CameraLoader.h"
#include "LightLoader.h"
#include "AudioLoader.h"
#include "ControllerLoader.h"
#include "GeneratorLoader.h"
#include "PhysicsLoader.h"
#include "ScriptLoader.h"

namespace loader {
    EntityLoader::EntityLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void EntityLoader::loadEntities(
        const YAML::Node& node,
        std::vector<EntityData>& entities,
        MaterialLoader& materialLoader,
        CustomMaterialLoader& customMaterialLoader,
        SpriteLoader& spriteLoader,
        CameraLoader& cameraLoader,
        LightLoader& lightLoader,
        AudioLoader& audioLoader,
        ControllerLoader& controllerLoader,
        GeneratorLoader& generatorLoader,
        PhysicsLoader& physicsLoader,
        ScriptLoader& scriptLoader) const
    {
        for (const auto& entry : node) {
            auto& data = entities.emplace_back();
            loadEntity(
                entry,
                data,
                materialLoader,
                customMaterialLoader,
                spriteLoader,
                cameraLoader,
                lightLoader,
                audioLoader,
                controllerLoader,
                generatorLoader,
                physicsLoader,
                scriptLoader);
        }
    }

    void EntityLoader::loadEntity(
        const YAML::Node& node,
        EntityData& data,
        MaterialLoader& materialLoader,
        CustomMaterialLoader& customMaterialLoader,
        SpriteLoader& spriteLoader,
        CameraLoader& cameraLoader,
        LightLoader& lightLoader,
        AudioLoader& audioLoader,
        ControllerLoader& controllerLoader,
        GeneratorLoader& generatorLoader,
        PhysicsLoader& physicsLoader,
        ScriptLoader& scriptLoader) const
    {
        loadEntityClone(
            node,
            data.base,
            data.clones,
            true,
            materialLoader,
            customMaterialLoader,
            spriteLoader,
            cameraLoader,
            lightLoader,
            audioLoader,
            controllerLoader,
            generatorLoader,
            physicsLoader,
            scriptLoader);
    }

    void EntityLoader::loadEntityClone(
        const YAML::Node& node,
        EntityCloneData& data,
        std::vector<EntityCloneData>& clones,
        bool recurse,
        MaterialLoader& materialLoader,
        CustomMaterialLoader& customMaterialLoader,
        SpriteLoader& spriteLoader,
        CameraLoader& cameraLoader,
        LightLoader& lightLoader,
        AudioLoader& audioLoader,
        ControllerLoader& controllerLoader,
        GeneratorLoader& generatorLoader,
        PhysicsLoader& physicsLoader,
        ScriptLoader& scriptLoader) const
    {
        bool hasClones = false;

        data.enabled = true;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "type") {
                std::string type = readString(v);
                if (type == "origo") {
                    data.type = mesh::EntityType::origo;
                }
                else if (type == "container") {
                    data.type = mesh::EntityType::container;
                }
                else if (type == "model") {
                    data.type = mesh::EntityType::model;
                }
                else if (type == "quad") {
                    data.type = mesh::EntityType::quad;
                }
                else if (type == "billboard") {
                    data.type = mesh::EntityType::billboard;
                }
                else if (type == "sprite") {
                    data.type = mesh::EntityType::sprite;
                }
                else if (type == "text") {
                    data.type = mesh::EntityType::text;
                }
                else if (type == "terrain") {
                    data.type = mesh::EntityType::terrain;
                }
                else {
                    reportUnknown("entity_type", k, v);
                }
            }
            else if (k == "name") {
                data.name = readString(v);
            }
			else if (k == "xxname" || k == "xname") {
				// NOTE quick disable logic
				data.name = readString(v);
				data.enabled = false;
			}
            else if (k == "active") {
                data.active = readBool(v);
            }
            else if (k == "priority") {
                data.priority = readInt(v);
            }
			else if (k == "desc") {
                data.desc = readString(v);
            }
            else if (k == "id") {
                data.idBase = readUUID(v);
            }
            else if (k == "parent_id") {
                data.parentIdBase = readUUID(v);
            }
            else if (k == "model") {
                if (v.Type() == YAML::NodeType::Sequence) {
                    data.meshPath = v[0].as<std::string>();
                    data.meshName = v[1].as<std::string>();
                }
                else {
                    data.meshName = readString(v);
                }
            }
            else if (k == "program" || k == "shader") {
                data.programName = readString(v);
                if (data.programName == "texture") {
                    data.programName = SHADER_TEXTURE;
                }
            }
            else if (k == "depth_program") {
                data.depthProgramName = readString(v);
            }
            else if (k == "geometry_type") {
                data.geometryType = readString(v);
            }
            else if (k == "program_definitions" || k == "shader_definitions") {
                for (const auto& defNode : v) {
                    const auto& defName = defNode.first.as<std::string>();
                    const auto& defValue = defNode.second.as<std::string>();
                    data.programDefinitions[util::toUpper(defName)] = defValue;
                }
            }
            else if (k == "render_flags") {
                for (const auto& flagNode : v) {
                    auto flagName = flagNode.first.as<std::string>();
                    const auto flagValue = readBool(flagNode.second);
                    data.renderFlags[util::toLower(flagName)] = flagValue;
                }
            }
            else if (k == "front") {
                data.front = readVec3(v);
            }
            else if (k == "font") {
                data.font.name = readString(v);
            }
            else if (k == "material") {
                data.materialName = readString(v);
            }
            else if (k == "material_modifier") {
                materialLoader.loadMaterialModifiers(v, data.materialModifiers);
            }
            else if (k == "force_material") {
                data.forceMaterial = readBool(v);
            }
            else if (k == "sprite") {
                data.spriteName = readString(v);
            }
            else if (k == "batch_size") {
                data.batchSize = readInt(v);
            }
            else if (k == "load_textures") {
                data.loadTextures = readBool(v);
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
            else if (k == "repeat") {
                loadRepeat(v, data.repeat);
            }
            else if (k == "tiling") {
                loadTiling(v, data.tiling);
            }
            else if (k == "camera") {
                cameraLoader.loadCamera(v, data.camera);
            }
            else if (k == "light") {
                lightLoader.loadLight(v, data.light);
            }
            else if (k == "audio") {
                audioLoader.loadAudio(v, data.audio);
            }
            else if (k == "custom_material") {
                customMaterialLoader.loadCustomMaterial(v, data.customMaterial);
            }
            else if (k == "physics") {
                physicsLoader.loadPhysics(v, data.physics);
            }
            else if (k == "controllers") {
                controllerLoader.loadControllers(v, data.controllers);
            }
            else if (k == "controller") {
                auto& controllerDaata = data.controllers.emplace_back();
                controllerLoader.loadController(v, controllerDaata);
            }
            else if (k == "generator") {
                generatorLoader.loadGenerator(v, data.generator);
            }
            else if (k == "instanced") {
                data.instanced = readBool(v);
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
                scriptLoader.loadScript(v, data.script);
            }
            else if (k == "script_file") {
                scriptLoader.loadScript(v, data.script);
            }
            else {
                reportUnknown("entity_entry", k, v);
            }
        }

        if (hasClones) {
            for (const auto& pair : node) {
                const std::string& k = pair.first.as<std::string>();
                const YAML::Node& v = pair.second;

                if (k == "clones") {
                    for (const auto& node : v) {
                        // NOTE KI intialize with current data
                        EntityCloneData clone = data;
                        std::vector<EntityCloneData> dummy{};
                        loadEntityClone(
                            node,
                            clone,
                            dummy,
                            false,
                            materialLoader,
                            customMaterialLoader,
                            spriteLoader,
                            cameraLoader,
                            lightLoader,
                            audioLoader,
                            controllerLoader,
                            generatorLoader,
                            physicsLoader,
                            scriptLoader);
                        clones.push_back(clone);
                    }
                }
            }
        }
    }
}
