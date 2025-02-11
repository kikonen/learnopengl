#include "MeshLoader.h"

#include <string>
#include <vector>
#include <algorithm>
#include <string>

#include <fmt/format.h>

#include "asset/Assets.h"

#include "util/util.h"
#include "util/glm_format.h"

#include "shader/Shader.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "registry/Registry.h"
#include "registry/ModelRegistry.h"

#include "Loaders.h"

#include "loader/document.h"
#include "loader_util.h"

namespace loader {
    MeshLoader::MeshLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void MeshLoader::loadMeshes(
        const loader::DocNode& node,
        std::vector<MeshData>& meshes,
        Loaders& loaders) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& data = meshes.emplace_back();
            loadMesh(entry, data, loaders);
        }
    }

    void MeshLoader::loadMesh(
        const loader::DocNode& node,
        MeshData& data,
        Loaders& loaders) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::DocNode& v = pair.getNode();

            const auto k = util::toLower(key);

            if (k == "enabled") {
                data.explicitEnabled = true;
                data.enabled = readBool(v);
            }
            else if (k == "id") {
                data.id = readString(v);
            }
            else if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = MeshDataType::none;
                }
                else if (type == "mesh") {
                    data.type = MeshDataType::mesh;
                }
                else if (type == "primitive") {
                    data.type = MeshDataType::primitive;
                }
                else if (type == "non_vao") {
                    data.type = MeshDataType::non_vao;
                }
                else {
                    reportUnknown("mesh_data_type", k, v);
                }
            }
            else if (k == "path" || k == "mesh") {
                data.path = readString(v);
                data.enabled = k != "xpath";

                if (data.baseDir.empty()) {
                    std::filesystem::path path{ data.path };
                    data.baseDir = path.parent_path().string();
                }
            }
            else if (k == "base_dir") {
                data.baseDir = readString(v);
            }
            else if (k == "animations") {
                loadAnimations(v, data.animations);
            }
            else if (k == "default_programs") {
                data.defaultPrograms = readBool(v);
            }
            else if (k == "program") {
                data.programs[MaterialProgramType::shader] = readString(v);
            }
            else if (k == "oit_program") {
                data.programs[MaterialProgramType::oit] = readString(v);
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
            else if (k == "materials") {
                loaders.m_materialLoader.loadMaterials(v, data.materials, loaders);
            }
            else if (k == "material") {
                if (data.materials.empty()) {
                    data.materials.emplace_back();
                }
                auto& materialData = data.materials[0];
                materialData.aliasName = "*";
                loaders.m_materialLoader.loadMaterial(v, materialData, loaders);
                materialData.materialName = materialData.material.m_name;
            }
            else if (k == "material_modifier") {
                if (data.materialModifiers.empty()) {
                    data.materialModifiers.emplace_back();
                }
                auto& materialData = data.materialModifiers[0];
                loaders.m_materialLoader.loadMaterialModifier(v, materialData, loaders);
            }
            else if (k == "material_modifiers") {
                loaders.m_materialLoader.loadMaterialModifiers(v, data.materialModifiers, loaders);
            }
            else if (k == "flags") {
                for (const auto& flagNode : v.getNodes()) {
                    const auto& flagName = flagNode.getName();
                    const auto& flagValue = readBool(flagNode.getNode());
                    data.meshFlags.set(util::toLower(flagName), flagValue);
                }
            }
            else if (k == "scale") {
                data.hasScale = true;
                data.scale = readScale3(v);
            }
            else if (k == "base_scale") {
                data.baseScale = readScale3(v);
            }
            else if (k == "base_rotation" || k == "base_rot") {
                data.hasBaseRotation = true;
                data.baseRotation = readVec3(v);
            }
            else if (k == "lod") {
                auto& lod = data.lods.emplace_back();
                lod.name = "*";
                loadLod(v, lod, loaders);
            }
            else if (k == "lods") {
                loadLods(v, data.lods, loaders);
            }
            else if (k == "sockets") {
                loadSockets(v, data.sockets);
            }
            else if (k == "vertex") {
                loaders.m_vertexLoader.load(v, data.vertexData);
            }
            else {
                reportUnknown("mesh_entry", k, v);
            }
        }

        if (data.name.empty()) {
            data.name = data.path;
        }

        if (data.explicitEnabled) {
            data.enabled &= data.type != MeshDataType::none;
        } else
        {
            if (data.vertexData.valid) {
                data.type = MeshDataType::primitive;
                data.enabled = true;
            }

            if (data.type == MeshDataType::non_vao) {
                data.enabled = true;
            }
        }
    }

    void MeshLoader::loadLods(
        const loader::DocNode& node,
        std::vector<LodData>& lods,
        Loaders& loaders) const
    {
        for (const auto& entry : node.getNodes()) {
            LodData& data = lods.emplace_back();
            loadLod(entry, data, loaders);
        }
    }

    void MeshLoader::loadLod(
        const loader::DocNode& node,
        LodData& data,
        Loaders& loaders) const
    {
        const auto& assets = Assets::get();
        data.minDistance = assets.nearPlane;
        data.maxDistance = assets.farPlane;

        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::DocNode& v = pair.getNode();

            const auto k = util::toLower(key);

            if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "min" || k == "min_distance") {
                data.minDistance = readFloat(v);
            }
            else if (k == "max" || k == "max_distance") {
                data.maxDistance = readFloat(v);
            }
            else if (k == "priority") {
                data.priority = readInt(v);
            }
            else if (k == "flags") {
                for (const auto& flagNode : v.getNodes()) {
                    const auto& flagName = flagNode.getName();
                    const auto& flagValue = readBool(flagNode.getNode());
                    data.meshFlags.set(util::toLower(flagName), flagValue);
                }
            }
            else if (k == "material_modifier") {
                if (data.materialModifiers.empty()) {
                    data.materialModifiers.emplace_back();
                }
                auto& materialData = data.materialModifiers[0];
                loaders.m_materialLoader.loadMaterialModifier(v, materialData, loaders);
            }
            else if (k == "material_modifiers") {
                loaders.m_materialLoader.loadMaterialModifiers(v, data.materialModifiers, loaders);
            }
            else {
                reportUnknown("lod_entry", k, v);
            }
        }
    }

    void MeshLoader::loadSockets(
        const loader::DocNode& node,
        std::vector<SocketData>& sockets) const
    {
        for (const auto& entry : node.getNodes()) {
            SocketData& data = sockets.emplace_back();
            loadSocket(entry, data);
        }
    }

    void MeshLoader::loadSocket(
        const loader::DocNode& node,
        SocketData& data) const
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
            else if (k == "joint") {
                data.joint = readString(v);
            }
            else if (k == "offset") {
                data.offset = readVec3(v);
            }
            else if (k == "rotation") {
                data.rotation = readDegreesRotation(v);
            }
            else if (k == "scale") {
                data.scale = readFloat(v);
            }
            else {
                reportUnknown("socket_entry", k, v);
            }
        }

        if (data.joint.empty()) {
            data.joint = data.name;
        }
        if (data.name.empty()) {
            data.name = data.joint;
        }
    }

    void MeshLoader::loadAnimations(
        const loader::DocNode& node,
        std::vector<AnimationData>& animations) const
    {
        for (const auto& entry : node.getNodes()) {
            AnimationData& data = animations.emplace_back();
            loadAnimation(entry, data);
        }
    }

    void MeshLoader::loadAnimation(
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
