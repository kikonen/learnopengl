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
            out << YAML::Key << "name";
            out << YAML::Value << meshSet->m_name;

            out << YAML::Key << "dir";
            out << YAML::Value << meshSet->m_dir;

            out << YAML::Key << "path";
            out << YAML::Value << meshSet->m_path;

            out << YAML::Key << "smooth_normals";
            out << YAML::Value << meshSet->m_smoothNormals;

            out << YAML::Key << "force_normals";
            out << YAML::Value << meshSet->m_forceNormals;

            out << YAML::Key << "root_dir";
            out << YAML::Value << meshSet->m_rootDir;

            out << YAML::Key << "file_path";
            out << YAML::Value << meshSet->m_filePath;
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
