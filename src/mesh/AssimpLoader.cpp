#include "AssimpLoader.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "util/Util.h"

#include "mesh/ModelMesh.h"

namespace mesh
{
    AssimpLoader::AssimpLoader(
        std::shared_ptr<std::atomic<bool>> alive)
        : ModelLoader{ alive }
    {}

    AssimpLoader::~AssimpLoader()
    {}

    void AssimpLoader::loadData(
        ModelMesh& mesh)
    {
        std::string filePath = util::joinPathExt(
            mesh.m_rootDir,
            mesh.m_meshPath,
            mesh.m_meshName, ".obj");

        KI_INFO(fmt::format("MESH_LOADER: path={}", filePath));

        if (!util::fileExists(filePath)) {
            throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", filePath) };
        }

        // Create an instance of the Importer class
        Assimp::Importer importer;

        // And have it read the given file with some example postprocessing
        // Usually - if speed is not the most important aspect for you - you'll
        // probably to request more postprocessing than we do in this example.
        const aiScene* scene = importer.ReadFile(
            filePath,
            aiProcess_GenNormals |
            aiProcess_GenSmoothNormals |
            aiProcess_FixInfacingNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_ImproveCacheLocality |
            aiProcess_LimitBoneWeights |
            aiProcess_RemoveRedundantMaterials |
            aiProcess_SortByPType);

        // If the import failed, report it
        if (nullptr == scene) {
            KI_ERROR(importer.GetErrorString());
            return;
        }

        // Now we can access the file's contents.
        processScene(mesh, scene);
    }

    void AssimpLoader::processScene(
        ModelMesh& mesh,
        const aiScene* scene)
    {
    }
}
