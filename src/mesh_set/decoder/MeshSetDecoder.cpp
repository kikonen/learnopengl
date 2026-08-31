#include "MeshSetDecoder.h"

#include "mesh/MeshSet.h"
#include "mesh/ModelMesh.h"

#include "ModelMeshDecoder.h"
#include "RigDecoder.h"

namespace
{
}


namespace mesh_set::decoder
{
    MeshSetDecoder::MeshSetDecoder() = default;
    MeshSetDecoder::~MeshSetDecoder() = default;

    void MeshSetDecoder::decode(
        const YAML::Node& node,
        const util::Ref<mesh::MeshSet>& meshSet)
    {
        if (!node) return;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const auto& v = pair.second;

            if (k == "id") {
                meshSet->m_id = v.as<std::string>();
                break;
            }
            if (k == "name") {
                meshSet->m_name = v.as<std::string>();
                break;
            }
            if (k == "dir") {
                meshSet->m_dir = v.as<std::string>();
                break;
            }
            if (k == "path") {
                meshSet->m_path = v.as<std::string>();
                break;
            }
            if (k == "smooth_normals") {
                meshSet->m_smoothNormals = v.as<bool>();
                break;
            }
            if (k == "force_normals") {
                meshSet->m_forceNormals = v.as<bool>();
                break;
            }
            if (k == "root_dir") {
                meshSet->m_rootDir = v.as<std::string>();
                break;
            }
            if (k == "file_path") {
                meshSet->m_filePath = v.as<std::string>();
                break;
            }
            if (k == "animation_paths") {
                decodeAnimationPaths(v, meshSet);
                break;
            }
        }
    }

    void MeshSetDecoder::decodeAnimationPaths(
        const YAML::Node& nodes,
        const util::Ref<mesh::MeshSet>& meshSet)
    {
        auto& animationPaths = meshSet->m_animationPaths;

        for (const auto& pathNode : nodes) {
            auto& animationPath = animationPaths.emplace_back();

            for (const auto& pair : pathNode) {
                const std::string& k = pair.first.as<std::string>();
                const auto& v = pair.second;

                if (k == "prefix") {
                    animationPath.animationPrefix = v.as<std::string>();
                    break;
                }
                if (k == "path") {
                    animationPath.path = v.as<std::string>();
                    break;
                }
            }
        }
    }
}
