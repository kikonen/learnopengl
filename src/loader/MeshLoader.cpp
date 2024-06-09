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
        int level = 0;
        for (const auto& entry : node.getNodes()) {
            auto& data = meshes.emplace_back();
            data.level = level;
            loadMesh(entry, data, loaders);
            level++;
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
                loaders.m_materialLoader.loadMaterialModifiers(v, materialData);
            }
            else if (k == "flags") {
                for (const auto& flagNode : v.getNodes()) {
                    const auto& flagName = flagNode.getName();
                    const auto& flagValue = readBool(flagNode.getNode());
                    data.meshFlags.set(util::toLower(flagName), flagValue);
                }
            }
            else {
                reportUnknown("model_entry", k, v);
            }
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
        int level = 0;
        for (const auto& entry : node.getNodes()) {
            LodData& data = lods.emplace_back();
            data.level = level;
            loadLod(entry, data);
            level++;
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
