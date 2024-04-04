#include "AssimpLoader.h"

#include <iostream>

#include <fmt/format.h>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "util/glm_format.h"
#include "util/Util.h"

#include "mesh/ModelMesh.h"

namespace {
    const std::string SUPPORTED_TYPES[] {
        ".dae",
        ".obj",
    };

    glm::vec4 toVec4(const aiColor4D& v) {
        return { v.r, v.g, v.b, v.a };
    }

    glm::vec3 toVec3(const aiVector3D& v) {
        return { v.x, v.y, v.z };
    }

    glm::vec2 toVec2(const aiVector3D& v) {
        return { v.x, v.y };
    }
}

namespace mesh
{
    AssimpLoader::AssimpLoader(
        std::shared_ptr<std::atomic<bool>> alive)
        : ModelLoader{ alive }
    {}

    AssimpLoader::~AssimpLoader()
    {}

    void AssimpLoader::loadData(
        ModelMesh& modelMesh)
    {
        for (const auto& ext : SUPPORTED_TYPES) {
            std::string filePath = util::joinPathExt(
                modelMesh.m_rootDir,
                modelMesh.m_meshPath,
                modelMesh.m_meshName, ext);

            if (util::fileExists(filePath)) {
                loadDataType(modelMesh, ext, filePath);
            }
        }
    }

    void AssimpLoader::loadDataType(
        ModelMesh& modelMesh,
        const std::string& fileExt,
        const std::string& filePath)
    {
        KI_INFO(fmt::format("MESH_LOADER: path={}", filePath));

        if (!util::fileExists(filePath)) {
            throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", filePath) };
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(
            filePath,
            aiProcess_GenNormals |
            //aiProcess_GenSmoothNormals |
            //aiProcess_FixInfacingNormals |
            aiProcess_CalcTangentSpace |
            //aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            //aiProcess_ImproveCacheLocality |
            aiProcess_LimitBoneWeights |
            //aiProcess_RemoveRedundantMaterials |
            aiProcess_SortByPType |
            0);

        // If the import failed, report it
        if (!scene) {
            KI_ERROR(importer.GetErrorString());
            return;
        }

        std::map<size_t, ki::material_id> materialMapping;

        processMaterials(modelMesh, materialMapping, scene);
        processNode(modelMesh, materialMapping, scene, scene->mRootNode);

        if (m_defaultMaterial.m_used) {
            modelMesh.m_materials.push_back(m_defaultMaterial);
        }
    }

    void AssimpLoader::processNode(
        ModelMesh& modelMesh,
        const std::map<size_t, ki::material_id>& materialMapping,
        const aiScene* scene,
        const aiNode* node)
    {
        for (size_t n = 0; n < node->mNumChildren; ++n)
        {
            processNode(modelMesh, materialMapping, scene, node->mChildren[n]);
        }

        for (size_t n = 0; n < node->mNumMeshes; ++n)
        {
            processMesh(modelMesh, materialMapping, node, scene->mMeshes[node->mMeshes[n]]);
        }
    }

    void AssimpLoader::processMesh(
        ModelMesh& modelMesh,
        const std::map<size_t, ki::material_id>& materialMapping,
        const aiNode* node,
        const aiMesh* mesh)
    {
        auto& vertices = modelMesh.m_vertices;
        vertices.reserve(mesh->mNumVertices);

        for (size_t vertexIndex = 0; vertexIndex  < mesh->mNumVertices; vertexIndex++) {
            glm::vec2 texCoord;

            if (mesh->HasTextureCoords(0))
            {
                texCoord = toVec2(mesh->mTextureCoords[0][vertexIndex]);
            }

            const auto pos = toVec3(mesh->mVertices[vertexIndex]);
            const auto normal = toVec3(mesh->mNormals[vertexIndex]);
            const auto tangent = toVec3(mesh->mTangents[vertexIndex]);

            ki::material_id materialId;
            {
                const auto& it = materialMapping.find(mesh->mMaterialIndex);
                materialId = it != materialMapping.end() ? it->second : 0;
            }

            if (m_forceDefaultMaterial || !materialId) {
                m_defaultMaterial.m_used = true;
                materialId = m_defaultMaterial.m_id;
            }

            vertices.emplace_back(pos, texCoord, normal, tangent, materialId);
        }

        for (size_t faceIdx = 0; faceIdx < mesh->mNumFaces; faceIdx++) {
            processFace(modelMesh, materialMapping, mesh, &mesh->mFaces[faceIdx]);
        }
    }

    void AssimpLoader::processFace(
        ModelMesh& modelMesh,
        const std::map<size_t, ki::material_id>& materialMapping,
        const aiMesh* mesh,
        const aiFace* face)
    {
        Index index{ 0, 0, 0 };
        for (size_t i = 0; i < face->mNumIndices; i++)
        {
            index[i] = face->mIndices[i];
        }
        modelMesh.m_indeces.push_back({ index });
    }

    void AssimpLoader::processMaterials(
        ModelMesh& modelMesh,
        std::map<size_t, ki::material_id>& materialMapping,
        const aiScene* scene)
    {
        auto& materials = modelMesh.m_materials;

        for (size_t n = 0; n < scene->mNumMaterials; ++n)
        {
            auto material = processMaterial(scene, scene->mMaterials[n]);
            materials.push_back(material);
            materialMapping.insert({ n, material.m_id });
        }
    }

    Material AssimpLoader::processMaterial(
        const aiScene* scene,
        const aiMaterial* material)
    {
        Material result;
        std::map<std::string, GLuint*> textureIdMap;

        int ret1, ret2;
        aiColor4D diffuse;
        aiColor4D specular;
        aiColor4D ambient;
        aiColor4D emission;
        ai_real shininess, strength;
        unsigned int max;

        int diffuseIndex = 0;
        int normalIndex = 0;
        aiString diffusePath;
        aiString normalPath;

        auto diffuseTexValid = material->GetTexture(aiTextureType_DIFFUSE, diffuseIndex, &diffusePath);
        auto normalTexValid = material->GetTexture(aiTextureType_NORMALS, normalIndex, &normalPath);

        auto diffuseValid = aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
        auto specularValid = aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular);
        auto ambientValid = aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient);
        auto emissionValid = aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &emission);

        max = 1;
        ret1 = aiGetMaterialFloatArray(material, AI_MATKEY_SHININESS, &shininess, &max);
        max = 1;
        ret2 = aiGetMaterialFloatArray(material, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);

        result.map_kd = diffusePath.C_Str();
        result.kd = toVec4(diffuse);
        result.ks = toVec4(specular);
        result.ka = toVec4(ambient);
        result.ke = toVec4(emission);

        result.ns = shininess;

        return result;
    }
}
