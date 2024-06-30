#include "MeshLoader.h"

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

            if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "path" || k == "mesh") {
                data.path = readString(v);

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
                loaders.m_materialLoader.loadMaterialModifiers(v, materialData);
            }
            else if (k == "flags") {
                for (const auto& flagNode : v.getNodes()) {
                    const auto& flagName = flagNode.getName();
                    const auto& flagValue = readBool(flagNode.getNode());
                    data.meshFlags.set(util::toLower(flagName), flagValue);
                }
            }
            else if (k == "lod") {
                auto& lod = data.lods.emplace_back();
                lod.name = "*";
                loadLod(v, lod);
            }
            else if (k == "lods") {
                loadLods(v, data.lods);
            }
            else if (k == "sockets") {
                loadSockets(v, data.sockets);
            }
            else {
                reportUnknown("mesh_entry", k, v);
            }
        }

        if (data.name.empty()) {
            data.name = data.path;
        }

        // NOTE KI ensure assigns are before modifiers
        std::sort(
            data.materials.begin(),
            data.materials.end(),
            [](auto& a, auto& b) { return a.modifier < b.modifier; });

        for (auto& materialData : data.materials) {
            loaders.m_materialLoader.resolveMaterialPaths(data.baseDir, materialData);
            loaders.m_materialLoader.resolveMaterialPbr(data.baseDir, materialData);
        }
    }

    void MeshLoader::loadLods(
        const loader::DocNode& node,
        std::vector<LodData>& lods) const
    {
        for (const auto& entry : node.getNodes()) {
            LodData& data = lods.emplace_back();
            data.levels = { {0} };
            data.name = '*';
            loadLod(entry, data);
        }
    }

    void MeshLoader::loadLod(
        const loader::DocNode& node,
        LodData& data) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::DocNode& v = pair.getNode();

            const auto k = util::toLower(key);

            if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "level") {
                data.levels = readIntVector(v, 1);
            }
            else if (k == "distance") {
                data.distance = readFloat(v);
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
                data.scale = readVec3(v);
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
