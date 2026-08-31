#include "MeshSetEncoder.h"

#include <algorithm>
#include <functional>
#include <set>

#include "mesh/MeshSet.h"
#include "mesh/ModelMesh.h"

#include "ModelMeshEncoder.h"
#include "RigEncoder.h"

namespace mesh_set::encoder
{
    MeshSetEncoder::MeshSetEncoder() = default;
    MeshSetEncoder::~MeshSetEncoder() = default;

    void MeshSetEncoder::encode(
        YAML::Emitter& out,
        const util::Ref<mesh::MeshSet>& meshSet)
    {
        out << YAML::BeginMap;

        {
            out << YAML::Key << "id";
            out << YAML::Value << meshSet->getId();

            out << YAML::Key << "name";
            out << YAML::Value << meshSet->getName();

            out << YAML::Key << "dir";
            out << YAML::Value << meshSet->getDir();

            out << YAML::Key << "path";
            out << YAML::Value << meshSet->getPath();

            out << YAML::Key << "smooth_normals";
            out << YAML::Value << meshSet->getSmoothNormals();

            out << YAML::Key << "force_normals";
            out << YAML::Value << meshSet->getForceNormals();

            out << YAML::Key << "root_dir";
            out << YAML::Value << meshSet->getRootDir();

            out << YAML::Key << "file_path";
            out << YAML::Value << meshSet->getFilePath();
        }

        {
            out << YAML::Key << "meshes";
            out << YAML::Value << YAML::BeginSeq;

            for (auto& mesh : meshSet->getMeshes()) {
                auto* modelMesh = dynamic_cast<mesh::ModelMesh*>(mesh.get());
                if (!modelMesh) continue;

                {
                    util::Ref<mesh::ModelMesh> ref{ modelMesh };
                    mesh_set::encoder::ModelMeshEncoder encoder;
                    encoder.encode(out, ref);
                }
            }

            out << YAML::EndSeq;
        }

        {
            out << YAML::Key << "animation_paths";
            out << YAML::Value << YAML::BeginSeq;

            for (const auto& animationPath : meshSet->getAnimationPaths()) {
                out << YAML::BeginMap;
                out << YAML::Key << "prefix";
                out << YAML::Value << animationPath.animationPrefix;
                out << YAML::Key << "path";
                out << YAML::Value << animationPath.path;
                out << YAML::EndMap;
            }

            out << YAML::EndSeq;
        }

        {
            out << YAML::Key << "rigs";
            out << YAML::Value << YAML::BeginSeq;

            std::set<const animation::Rig*> processedRigs;

            for (auto& mesh : meshSet->getMeshes()) {
                auto* modelMesh = dynamic_cast<mesh::ModelMesh*>(mesh.get());
                if (!modelMesh) continue;

                const auto& rig = modelMesh->getRig();
                if (!rig) continue;

                if (processedRigs.contains(rig.get())) continue;
                processedRigs.insert(rig.get());

                processedRigs.insert({ rig.get() });

                mesh_set::encoder::RigEncoder encoder;
                encoder.encode(out, rig);
            }

            out << YAML::EndSeq;
        }

        out << YAML::EndMap;
    }
}
