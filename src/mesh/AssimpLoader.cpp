#include "AssimpLoader.h"

#include <iostream>
#include <filesystem>
#include <mutex>

#include <fmt/format.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "util/glm_format.h"
#include "util/Util.h"

#include "animation/RigContainer.h"
#include "animation/RigNode.h"
#include "animation/Animation.h"
#include "animation/BoneChannel.h"
#include "animation/BoneContainer.h"
#include "animation/BoneInfo.h"
#include "animation/AnimationLoader.h"

#include "mesh/ModelMesh.h"

#include "util/assimp_util.h"

namespace {
    std::mutex m_lock;
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
        std::string filePath = assimp_util::resolvePath(
            modelMesh.m_rootDir,
            modelMesh.m_meshPath);

        if (filePath.empty()) return;

        modelMesh.m_filePath = filePath;
        loadResolvedPath(modelMesh);
    }

    void AssimpLoader::loadResolvedPath(
        ModelMesh& modelMesh)
    {
        std::lock_guard lock(m_lock);

        KI_INFO_OUT(fmt::format("ASSIMP: FILE path={}", modelMesh.m_filePath));

        if (!util::fileExists(modelMesh.m_filePath)) {
            throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", modelMesh.m_filePath) };
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(
            modelMesh.m_filePath,
            //aiProcess_GenNormals |
            aiProcess_GenSmoothNormals |
            aiProcess_ForceGenNormals |
            //aiProcess_FixInfacingNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            //aiProcess_ImproveCacheLocality |
            aiProcess_LimitBoneWeights |
            aiProcess_RemoveRedundantMaterials |
            aiProcess_GenUVCoords |
            aiProcess_SortByPType |
            0);

        // If the import failed, report it
        if (!scene) {
            KI_ERROR(importer.GetErrorString());
            return;
        }

        KI_INFO_OUT(fmt::format(
            "ASSIMP: SCENE scene={}, meshes={}, anims={}, materials={}, textures={}",
            modelMesh.m_filePath,
            scene->mNumMeshes,
            scene->mNumAnimations,
            scene->mNumMaterials,
            scene->mNumTextures));

        modelMesh.m_rig = std::make_unique<animation::RigContainer>();
        auto& rig = *modelMesh.m_rig;

        processMaterials(rig.m_materialMapping, modelMesh, scene);

        std::vector<const aiNode*> assimpNodes;
        collectNodes(rig, assimpNodes, scene, scene->mRootNode, -1, glm::mat4{ 1.f });
        //rig.calculateInvTransforms();

        processMeshes(
            rig,
            assimpNodes,
            modelMesh,
            scene);

        loadAnimations(rig, modelMesh.m_meshName, scene);

        if (m_defaultMaterial.m_used) {
            modelMesh.m_materials.push_back(m_defaultMaterial);
        }
    }

    void AssimpLoader::collectNodes(
        animation::RigContainer& rig,
        std::vector<const aiNode*>& assimpNodes,
        const aiScene* scene,
        const aiNode* node,
        int16_t parentIndex,
        const glm::mat4& parentTransform)
    {
        //glm::mat4 transform;
        uint16_t nodeIndex;
        {
            assimpNodes.push_back(node);

            auto& rigNode = rig.addNode(node);
            //rigNode.m_globalTransform = parentTransform * rigNode.m_localTransform;
            rigNode.m_parentIndex = parentIndex;
            nodeIndex = rigNode.m_index;

            //transform = rigNode.m_globalTransform;
        }

        KI_INFO_OUT(fmt::format("ASSIMP: NODE parent={}, node={}, name={}, children={}, meshes={}",
            parentIndex,
            nodeIndex,
            node->mName.C_Str(),
            node->mNumChildren,
            node->mNumMeshes));

        for (size_t n = 0; n < node->mNumChildren; ++n)
        {
            collectNodes(rig, assimpNodes, scene, node->mChildren[n], nodeIndex, glm::mat4{ 1.f });
        }
    }

    void AssimpLoader::loadAnimations(
        animation::RigContainer& rig,
        const std::string& namePrefix,
        const aiScene* scene)
    {
        animation::AnimationLoader loader{};

        loader.loadAnimations(
            rig,
            namePrefix,
            scene);
    }

    void AssimpLoader::processMeshes(
        animation::RigContainer& rig,
        const std::vector<const aiNode*>& assimpNodes,
        ModelMesh& modelMesh,
        const aiScene* scene)
    {
        std::vector<glm::mat4> globalTransforms;
        globalTransforms.resize(rig.m_nodes.size());

        for (auto& rigNode : rig.m_nodes) {
            const glm::mat4& parentTransform = rigNode.m_parentIndex >= 0 ? globalTransforms[rigNode.m_parentIndex] : glm::mat4(1.f);
            globalTransforms[rigNode.m_index] = parentTransform * rigNode.m_localTransform;

            auto& node = assimpNodes[rigNode.m_index];
            if (node->mNumMeshes == 0) continue;

            // TODO KI *HOW* logic when meshes are for LODs and when they are
            // required for model
            // - linden_tree = multiple plane meshes with same material
            // - texture_cube_4/airbnoat = separate meshes per material
            // - lion = multiple LOD meshes, but for each LOD extra material mesh (which can be ignored likely)
            if (false && !modelMesh.m_vertices.empty()) {
                KI_INFO_OUT(fmt::format("ASSIMP: SKIP node={}, meshes={}",
                    rigNode.m_index,
                    node->mNumMeshes));
                continue;
            }

            {
                modelMesh.m_transform = globalTransforms[rigNode.m_index];

                auto from = std::min((unsigned int)0, node->mNumMeshes - 1);
                auto count = std::min((unsigned int)10, node->mNumMeshes - from);
                for (size_t meshIndex = from; meshIndex < count; ++meshIndex)
                {
                    processMesh(
                        rig,
                        rigNode,
                        modelMesh,
                        meshIndex,
                        scene->mMeshes[node->mMeshes[meshIndex]]);
                }
            }
        }
    }

    void AssimpLoader::processMesh(
        animation::RigContainer& rig,
        animation::RigNode& rigNode,
        ModelMesh& modelMesh,
        size_t meshIndex,
        const aiMesh* mesh)
    {
        auto& vertices = modelMesh.m_vertices;
        const auto vertexOffset = vertices.size();
        vertices.reserve(vertexOffset + mesh->mNumVertices);

        Material* mat = nullptr;
        ki::material_id materialId;
        {
            const auto& it = rig.m_materialMapping.find(mesh->mMaterialIndex);
            materialId = it != rig.m_materialMapping.end() ? it->second : 0;
            mat = Material::findID(materialId, modelMesh.m_materials);
        }

        if (m_forceDefaultMaterial || !materialId) {
            m_defaultMaterial.m_used = true;
            materialId = m_defaultMaterial.m_id;
        }

        KI_INFO_OUT(fmt::format("ASSIMP: MESH node={}, name={}, offset={}, material={}, vertices={}, faces={}, bones={}",
            rigNode.m_index,
            mesh->mName.C_Str(),
            vertexOffset,
            mat ? mat->m_name : fmt::format("{}", mesh->mMaterialIndex),
            mesh->mNumVertices,
            mesh->mNumFaces,
            mesh->mNumBones));

        for (size_t vertexIndex = 0; vertexIndex  < mesh->mNumVertices; vertexIndex++) {
            glm::vec2 texCoord;

            if (mesh->HasTextureCoords(0))
            {
                texCoord = assimp_util::toVec2(mesh->mTextureCoords[0][vertexIndex]);
            }

            const auto pos = assimp_util::toVec3(mesh->mVertices[vertexIndex]);
            glm::vec3 normal{ 0.f };
            glm::vec3 tangent{ 0.f };

            if (mesh->mNormals) {
                normal = assimp_util::toVec3(mesh->mNormals[vertexIndex]);
            }
            if (mesh->mTangents) {
                tangent = assimp_util::toVec3(mesh->mTangents[vertexIndex]);
            }

            //KI_INFO_OUT(fmt::format("ASSIMP: offset={}, pos={}", vertexOffset, pos));

            vertices.emplace_back(pos, texCoord, normal, tangent, materialId);
        }

        for (size_t faceIdx = 0; faceIdx < mesh->mNumFaces; faceIdx++) {
            processMeshFace(rig, modelMesh, meshIndex, faceIdx, vertexOffset, mesh, &mesh->mFaces[faceIdx]);
        }

        for (size_t boneIdx = 0; boneIdx < mesh->mNumBones; boneIdx++) {
            processMeshBone(rig, modelMesh, meshIndex, vertexOffset, mesh, mesh->mBones[boneIdx]);
        }
    }

    void AssimpLoader::processMeshFace(
        animation::RigContainer& rig,
        ModelMesh& modelMesh,
        size_t meshIndex,
        size_t faceIndex,
        size_t vertexOffset,
        const aiMesh* mesh,
        const aiFace* face)
    {
        Index index{ 0, 0, 0 };
        for (uint32_t i = 0; i < face->mNumIndices; i++)
        {
            // NOTE KI multi material models are split by *material* into meshes
            // vertices and thus indeces areare per mesh, but they are *combined*
            // back single mesh in load
            // => must apply vertex offset in index buffer to match that
            index[i] = static_cast<glm::uint>(face->mIndices[i] + vertexOffset);
        }
        //KI_INFO_OUT(fmt::format("ASSIMP: FACE mesh={}, face={}, offset={}, idx={}",
        //    mesh->mName.C_Str(),
        //    faceIndex,
        //    vertexOffset,
        //    index));
        modelMesh.m_indeces.push_back({ index });
    }

    void AssimpLoader::processMeshBone(
        animation::RigContainer& rig,
        ModelMesh& modelMesh,
        size_t meshIndex,
        size_t vertexOffset,
        const aiMesh* mesh,
        const aiBone* bone)
    {
        auto& bi = rig.m_boneContainer.registerBone(bone);
        auto* rigNode = rig.findNode(bi.m_nodeName);
        if (rigNode) {
            rig.m_boneContainer.bindNode(bi.m_index, rigNode->m_index);
        }

        KI_INFO_OUT(fmt::format(
            "ASSIMP: BONE node={}, bone={}, name={}, mesh={}, offset={}, weights={}",
            bi.m_nodeIndex,
            bi.m_index,
            bi.m_nodeName,
            meshIndex,
            vertexOffset,
            bone->mNumWeights))


        auto& vertexBones = rig.m_vertexBones;

        Index index{ 0, 0, 0 };
        for (size_t i = 0; i < bone->mNumWeights; i++)
        {
            const auto& vw = bone->mWeights[i];
            const auto mat = assimp_util::toMat4(bone->mOffsetMatrix);

            //KI_INFO_OUT(fmt::format(
            //    "ASSIMP: mesh={}, bone={}, vertex={}, weight={}",
            //    meshIndex,
            //    boneIndex,
            //    weight->mVertexId,
            //    weight->mWeight));

            auto vertexIndex = vertexOffset + bone->mWeights[i].mVertexId;

            vertexBones.resize(std::max(vertexIndex + 1, vertexBones.size()));
            vertexBones[vertexIndex].addBone(bi.m_index, vw.mWeight);
        }
    }

    void AssimpLoader::processMaterials(
        std::map<size_t, ki::material_id>& materialMapping,
        ModelMesh& modelMesh,
        const aiScene* scene)
    {
        auto& materials = modelMesh.m_materials;

        for (size_t n = 0; n < scene->mNumMaterials; ++n)
        {
            auto material = processMaterial(modelMesh, scene, scene->mMaterials[n]);
            materials.push_back(material);
            materialMapping.insert({ n, material.m_id });
        }
    }

    Material AssimpLoader::processMaterial(
        ModelMesh& modelMesh,
        const aiScene* scene,
        const aiMaterial* src)
    {
        const auto name = src->GetName().C_Str();

        KI_INFO_OUT(fmt::format("ASSIMP: MATERIAL name={}, properties={}, allocated={}",
            name,
            src->mNumProperties,
            src->mNumAllocated));

        Material material;
        material.m_name = name;

        {
            aiColor4D diffuse;

            if (aiGetMaterialColor(src, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == AI_SUCCESS)
            {
                material.kd = assimp_util::toVec4(diffuse);

                float diffuseAlpha;
                if (aiGetMaterialFloat(src, AI_MATKEY_OPACITY, &diffuseAlpha) == AI_SUCCESS) {
                    material.kd.a = diffuseAlpha;
                }
            }
        }
        {
            aiColor4D specular;
            if (aiGetMaterialColor(src, AI_MATKEY_COLOR_SPECULAR, &specular) == AI_SUCCESS) {
                material.ks = assimp_util::toVec4(specular);
            }
        }

        {
            aiColor4D ambient;
            if (aiGetMaterialColor(src, AI_MATKEY_COLOR_AMBIENT, &ambient) == AI_SUCCESS) {
                material.ka = assimp_util::toVec4(ambient);
            }
        }
        {
            aiColor4D emission;
            if (aiGetMaterialColor(src, AI_MATKEY_COLOR_EMISSIVE, &emission) == AI_SUCCESS) {
                material.ke = assimp_util::toVec4(emission);
            }
        }

        {
            ai_real shininess;
            if (aiGetMaterialFloat(src, AI_MATKEY_SHININESS, &shininess) == AI_SUCCESS) {
                material.ns = shininess;
            }

            ai_real strength;
            if (aiGetMaterialFloat(src, AI_MATKEY_SHININESS_STRENGTH, &strength) == AI_SUCCESS) {
                //material.ns = shininess;
            }
        }

        {
            int diffuseIndex = 0;
            aiString diffusePath;

            if (src->GetTexture(aiTextureType_DIFFUSE, diffuseIndex, &diffusePath) == AI_SUCCESS) {
                //auto* embedded = scene->GetEmbeddedTexture(diffusePath.C_Str());
                material.map_kd = findTexturePath(modelMesh, diffusePath.C_Str());
            }
        }
        {
            int bumpIndex = 0;
            aiString bumpPath;
            if (src->GetTexture(aiTextureType_HEIGHT, bumpIndex, &bumpPath) == AI_SUCCESS) {
                material.map_bump = findTexturePath(modelMesh, bumpPath.C_Str());
            }
        }
        {
            int normalIndex = 0;
            aiString normalPath;
            if (src->GetTexture(aiTextureType_NORMALS, normalIndex, &normalPath) == AI_SUCCESS) {
                material.map_bump = findTexturePath(modelMesh, normalPath.C_Str());
            }
        }
        {
            int emissionIndex = 0;
            aiString emissionPath;
            if (src->GetTexture(aiTextureType_EMISSIVE, emissionIndex, &emissionPath) == AI_SUCCESS) {
                material.map_ke = findTexturePath(modelMesh, emissionPath.C_Str());
            }
        }

        return material;
    }

    std::string AssimpLoader::findTexturePath(
        ModelMesh& modelMesh,
        std::string origPath)
    {
        std::string assetPath = origPath;
        std::filesystem::path meshPath{ modelMesh.m_meshName };
        const auto parentPath = meshPath.parent_path();

        std::filesystem::path fsPath{ assetPath };
        std::string assetPath2 = std::filesystem::weakly_canonical(fsPath).string();

        std::string filePath = util::joinPathExt(
            modelMesh.m_rootDir,
            parentPath.string(),
            assetPath, "");

        if (util::fileExists(filePath)) {
            //assetPath = util::joinPath(
            //    parentPath.filename().string(),
            //    assetPath);
            assetPath = filePath.substr(
                modelMesh.m_rootDir.length() + 1,
                filePath.length() - modelMesh.m_rootDir.length() - 1);
        }

        KI_INFO_OUT(fmt::format("ASSIMP: TEX path={}, was={}", assetPath, origPath));

        return assetPath;
    }
}
