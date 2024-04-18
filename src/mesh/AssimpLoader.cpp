#include "AssimpLoader.h"

#include <iostream>
#include <filesystem>
#include <mutex>

#include <fmt/format.h>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "util/glm_format.h"
#include "util/Util.h"

#include "animation/AnimationContainer.h"
#include "animation/AnimationNode.h"
#include "animation/BoneContainer.h"

#include "mesh/ModelMesh.h"

#include "util/assimp_util.h"

namespace {
    std::mutex m_lock;

    const std::string SUPPORTED_TYPES[] {
        ".glb",
        ".fbx",
        ".dae",
        ".obj",
    };
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
        {
            std::string filePath = util::joinPathExt(
                modelMesh.m_rootDir,
                modelMesh.m_meshPath,
                modelMesh.m_meshName, "");

            if (util::fileExists(filePath)) {
                modelMesh.m_filePath = filePath;
                modelMesh.m_fileExt = "";
                loadResolvedPath(modelMesh);
                return;
            }
        }

        // NOTE KI process in priority order; stop at first match
        for (const auto& ext : SUPPORTED_TYPES) {
            std::string filePath = util::joinPathExt(
                modelMesh.m_rootDir,
                modelMesh.m_meshPath,
                modelMesh.m_meshName, ext);

            if (util::fileExists(filePath)) {
                modelMesh.m_filePath = filePath;
                modelMesh.m_fileExt = ext;
                loadResolvedPath(modelMesh);
                return;
            }
        }
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
            "ASSIMP: SCENE scene={}, meshes={}, anims={}, skeletons={}, materials={}, textures={}",
            modelMesh.m_filePath,
            scene->mNumMeshes,
            scene->mNumAnimations,
            scene->mNumSkeletons,
            scene->mNumMaterials,
            scene->mNumTextures));

        animation::AnimationContainer animContainer;

        processMaterials(animContainer.m_materialMapping, modelMesh, scene);

        collectNodes(animContainer, scene, scene->mRootNode, -1, glm::mat4{ 1.f });

        processMeshes(
            animContainer,
            modelMesh,
            scene);

        if (m_defaultMaterial.m_used) {
            modelMesh.m_materials.push_back(m_defaultMaterial);
        }

        for (size_t skeletonIndex = 0; skeletonIndex < scene->mNumSkeletons; ++skeletonIndex)
        {
            processSkeleton(
                animContainer,
                modelMesh,
                skeletonIndex,
                scene,
                scene->mSkeletons[skeletonIndex]);
        }

        for (size_t animIndex = 0; animIndex < scene->mNumAnimations; ++animIndex)
        {
            processAnimation(
                animContainer,
                modelMesh,
                animIndex,
                scene,
                scene->mAnimations[animIndex]);
        }
    }

    void AssimpLoader::processSkeleton(
        animation::AnimationContainer& animContainer,
        ModelMesh& modelMesh,
        size_t skeletonIndex,
        const aiScene* scene,
        const aiSkeleton* skeleton)
    {
        KI_INFO_OUT(fmt::format(
            "ASSIMP: SKELETON skeleton={}, index={}, bones={}",
            skeleton->mName.C_Str(),
            skeletonIndex,
            skeleton->mNumBones));

        for (size_t boneIndex = 0; boneIndex < skeleton->mNumBones; ++boneIndex)
        {
            processSkeletonBone(
                animContainer,
                modelMesh,
                skeletonIndex,
                boneIndex,
                scene,
                skeleton,
                skeleton->mBones[boneIndex]);
        }
    }

    void AssimpLoader::processSkeletonBone(
        animation::AnimationContainer& animContainer,
        ModelMesh& modelMesh,
        size_t skeletonIndex,
        size_t boneIndex,
        const aiScene* scene,
        const aiSkeleton* skeleton,
        const aiSkeletonBone* bone)
    {
        KI_INFO_OUT(fmt::format(
            "ASSIMP: SKELETON bone={}, parent={}, bone={}, node={}, mesh={}",
            skeletonIndex,
            bone->mParent,
            boneIndex,
            bone->mNode->mName.C_Str(),
            bone->mMeshId->mName.C_Str()));
    }

    void AssimpLoader::processAnimation(
        animation::AnimationContainer& animContainer,
        ModelMesh& modelMesh,
        size_t animIndex,
        const aiScene* scene,
        const aiAnimation* anim)
    {
        KI_INFO_OUT(fmt::format(
            "ASSIMP: ANIM anim={}, index={}, duration={}, ticksPerSec={}, channels={}",
            anim->mName.C_Str(),
            animIndex,
            anim->mDuration,
            anim->mTicksPerSecond,
            anim->mNumChannels));

        for (size_t channelIdx = 0; channelIdx < anim->mNumChannels; ++channelIdx)
        {
            const aiNodeAnim* channel = anim->mChannels[channelIdx];
            KI_INFO_OUT(fmt::format(
                "ASSIMP: CHANNEL anim={}, index={}, channel={}, node={}, posKeys={}, rotKeys={}, scalingKeys={}",
                anim->mName.C_Str(),
                animIndex,
                channelIdx,
                channel->mNodeName.C_Str(),
                channel->mNumPositionKeys,
                channel->mNumRotationKeys,
                channel->mNumScalingKeys));
        }
    }

    void AssimpLoader::collectNodes(
        animation::AnimationContainer& animContainer,
        const aiScene* scene,
        const aiNode* node,
        int16_t parentId,
        const glm::mat4& parentTransform)
    {
        const auto transform = parentTransform * assimp_util::toMat4(node->mTransformation);
        auto& animNode = animContainer.addNode(node);
        animNode.m_transform = transform;
        animNode.m_parentId = parentId;

        KI_INFO_OUT(fmt::format("ASSIMP: NODE parent={}, node={}, name={}, children={}, meshes={}",
            parentId,
            animNode.m_id,
            node->mName.C_Str(),
            node->mNumChildren,
            node->mNumMeshes));

        for (size_t n = 0; n < node->mNumChildren; ++n)
        {
            collectNodes(animContainer, scene, node->mChildren[n], animNode.m_id, transform);
        }
    }

    void AssimpLoader::processMeshes(
        animation::AnimationContainer& animContainer,
        ModelMesh& modelMesh,
        const aiScene* scene)
    {
        for (auto& animNode : animContainer.m_nodes) {
            auto& node = animNode.m_node;
            if (node->mNumMeshes == 0) continue;

            // TODO KI *HOW* logic when meshes are for LODs and when they are
            // required for model
            // - linden_tree = multiple plane meshes with same material
            // - texture_cube_4/airbnoat = separate meshes per material
            // - lion = multiple LOD meshes, but for each LOD extra material mesh (which can be ignored likely)
            if (false && !modelMesh.m_vertices.empty()) {
                KI_INFO_OUT(fmt::format("ASSIMP: SKIP node={}, meshes={}",
                    animNode.m_id,
                    node->mNumMeshes));
                continue;
            }

            {
                modelMesh.m_transform = animNode.m_transform;

                auto from = std::min((unsigned int)0, node->mNumMeshes - 1);
                auto count = std::min((unsigned int)10, node->mNumMeshes - from);
                for (size_t meshIndex = from; meshIndex < count; ++meshIndex)
                {
                    processMesh(
                        animContainer,
                        animNode,
                        modelMesh,
                        meshIndex,
                        scene->mMeshes[node->mMeshes[meshIndex]]);
                }
            }
        }
    }

    void AssimpLoader::processMesh(
        animation::AnimationContainer& animContainer,
        animation::AnimationNode& animNode,
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
            const auto& it = animContainer.m_materialMapping.find(mesh->mMaterialIndex);
            materialId = it != animContainer.m_materialMapping.end() ? it->second : 0;
            mat = Material::findID(materialId, modelMesh.m_materials);
        }

        if (m_forceDefaultMaterial || !materialId) {
            m_defaultMaterial.m_used = true;
            materialId = m_defaultMaterial.m_id;
        }

        KI_INFO_OUT(fmt::format("ASSIMP: MESH node={}, name={}, offset={}, material={}, vertices={}, faces={}, bones={}",
            animNode.m_id,
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
            processMeshFace(animContainer, animNode, modelMesh, meshIndex, faceIdx, vertexOffset, mesh, &mesh->mFaces[faceIdx]);
        }

        for (size_t boneIdx = 0; boneIdx < mesh->mNumBones; boneIdx++) {
            processMeshBone(animContainer, animNode, modelMesh, meshIndex, boneIdx, vertexOffset, mesh, mesh->mBones[boneIdx]);
        }
    }

    void AssimpLoader::processMeshFace(
        animation::AnimationContainer& animContainer,
        animation::AnimationNode& animNode,
        ModelMesh& modelMesh,
        size_t meshIndex,
        size_t faceIndex,
        size_t vertexOffset,
        const aiMesh* mesh,
        const aiFace* face)
    {
        Index index{ 0, 0, 0 };
        for (size_t i = 0; i < face->mNumIndices; i++)
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
        animation::AnimationContainer& animContainer,
        animation::AnimationNode& animNode,
        ModelMesh& modelMesh,
        size_t meshIndex,
        size_t boneIndex,
        size_t vertexOffset,
        const aiMesh* mesh,
        const aiBone* bone)
    {
        const auto offsetMatrix = assimp_util::toMat4(bone->mOffsetMatrix);

        KI_INFO_OUT(fmt::format(
            "ASSIMP: BONE node={}, bone={}, name={}, mesh={}, offset={}, weights={}, offsetMatrix={}",
            animNode.m_id,
            boneIndex,
            bone->mName.C_Str(),
            meshIndex,
            vertexOffset,
            bone->mNumWeights,
            offsetMatrix))

        auto boneId = modelMesh.m_boneContainer.getBoneId(bone);

        auto& vertexBones = modelMesh.m_vertexBones;

        Index index{ 0, 0, 0 };
        for (size_t i = 0; i < bone->mNumWeights; i++)
        {
            const auto& vw = bone->mWeights[i];
            const auto mat = assimp_util::toMat4(bone->mOffsetMatrix);

            //KI_INFO_OUT(fmt::format(
            //    "ASSIMP: mesh={}, bone={}, vertex={}, weight={}, mat={}",
            //    meshIndex, boneIndex,
            //    weight->mVertexId, weight->mWeight, mat));

            auto vertexIndex = vertexOffset + bone->mWeights[i].mVertexId;

            vertexBones.resize(std::max(vertexIndex + 1, vertexBones.size()));
            vertexBones[vertexIndex].addBone(boneId, vw.mWeight);
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
        const aiMaterial* material)
    {
        KI_INFO_OUT(fmt::format("ASSIMP: MATERIAL name={}, properties={}, allocated={}",
            material->GetName().C_Str(),
            material->mNumProperties,
            material->mNumAllocated));

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
        int bumpIndex = 0;
        int normalIndex = 0;
        int emissionIndex = 0;
        aiString diffusePath;
        aiString bumpPath;
        aiString normalPath;
        aiString emissionPath;

        auto diffuseTexValid = material->GetTexture(aiTextureType_DIFFUSE, diffuseIndex, &diffusePath);
        auto bumpTexValid = material->GetTexture(aiTextureType_HEIGHT, bumpIndex, &bumpPath);
        auto normalTexValid = material->GetTexture(aiTextureType_NORMALS, normalIndex, &normalPath);
        auto emissionTexValid = material->GetTexture(aiTextureType_EMISSIVE, emissionIndex, &emissionPath);

        auto diffuseValid = aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
        auto specularValid = aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular);
        auto ambientValid = aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient);
        auto emissionValid = aiGetMaterialColor(material, AI_MATKEY_COLOR_EMISSIVE, &emission);

        max = 1;
        ret1 = aiGetMaterialFloatArray(material, AI_MATKEY_SHININESS, &shininess, &max);
        max = 1;
        ret2 = aiGetMaterialFloatArray(material, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);

        if (diffuseTexValid == AI_SUCCESS) {
            //auto* embedded = scene->GetEmbeddedTexture(diffusePath.C_Str());
            result.map_kd = findTexturePath(modelMesh, diffusePath.C_Str());
        }
        if (bumpTexValid == AI_SUCCESS) {
            result.map_bump = findTexturePath(modelMesh, bumpPath.C_Str());
        }
        if (normalTexValid == AI_SUCCESS) {
            result.map_bump = findTexturePath(modelMesh, normalPath.C_Str());
        }
        if (emissionTexValid == AI_SUCCESS) {
            result.map_ke = findTexturePath(modelMesh, emissionPath.C_Str());
        }

        result.kd = assimp_util::toVec4(diffuse);
        result.ks = assimp_util::toVec4(specular);
        result.ka = assimp_util::toVec4(ambient);
        result.ke = assimp_util::toVec4(emission);

        result.ns = shininess;

        result.m_name = material->GetName().C_Str();

        return result;
    }

    std::string AssimpLoader::findTexturePath(
        ModelMesh& modelMesh,
        std::string assetPath)
    {
        std::filesystem::path meshPath{ modelMesh.m_meshName };
        const auto parentPath = meshPath.parent_path();

        //std::filesystem::path path{ modelMesh.m_filePath };

        std::string filePath = util::joinPathExt(
            modelMesh.m_rootDir,
            parentPath.string(),
            assetPath, "");

        if (util::fileExists(filePath)) {
            assetPath = util::joinPath(
                parentPath.filename().string(),
                assetPath);
        }

        KI_INFO_OUT(fmt::format("ASSIMP: TEX path={}", assetPath));

        return assetPath;
    }
}
