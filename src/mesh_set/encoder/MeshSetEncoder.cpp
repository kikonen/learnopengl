#include "MeshSetEncoder.h"

#include <algorithm>
#include <functional>

#include "mesh/MeshSet.h"
#include "mesh/ModelMesh.h"

#include "ModelMeshEncoder.h"

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
                if (modelMesh) {
                    util::Ref<mesh::ModelMesh> ref{ modelMesh };
                    mesh_set::encoder::ModelMeshEncoder encoder;
                    encoder.encode(out, ref);
                }
            }

            out << YAML::EndSeq;
        }

        //if (meshSet->m_refCount) {

        //}

        out << YAML::EndMap;
    }
}
