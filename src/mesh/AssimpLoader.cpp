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

#include "mesh/ModelMesh.h"

namespace {
    std::mutex m_lock;

    const std::string SUPPORTED_TYPES[] {
        ".glb",
        ".fbx",
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

    glm::mat4 toMat4(const aiMatrix4x4 & v) {
        return {
            v.a1, v.b1, v.c1, v.d1,
            v.a2, v.b2, v.c2, v.d2,
            v.a3, v.b3, v.c3, v.d3,
            v.a4, v.b4, v.c4, v.d4,
        };
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

        KI_INFO_OUT(fmt::format("ASSIMP_LOADER: path={}", modelMesh.m_filePath));

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
            "SCENE: name={}, meshes={}, anims={}, skeletons={}, materials={}, textures={}",
            scene->mName.C_Str(),
            scene->mNumMeshes,
            scene->mNumAnimations,
            scene->mNumSkeletons,
            scene->mNumMaterials,
            scene->mNumTextures));

        std::map<size_t, ki::material_id> materialMapping;

        processMaterials(modelMesh, materialMapping, scene);
        processNode(modelMesh, materialMapping, scene, scene->mRootNode, 0);

        if (m_defaultMaterial.m_used) {
            modelMesh.m_materials.push_back(m_defaultMaterial);
        }

        for (size_t skeletonIndex = 0; skeletonIndex < scene->mNumSkeletons; ++skeletonIndex)
        {
            processSkeleton(
                modelMesh,
                skeletonIndex,
                scene,
                scene->mSkeletons[skeletonIndex]);
        }

        for (size_t animIndex = 0; animIndex < scene->mNumAnimations; ++animIndex)
        {
            processAnimation(
                modelMesh,
                animIndex,
                scene,
                scene->mAnimations[animIndex]);
        }
    }

    void AssimpLoader::processSkeleton(
        ModelMesh& modelMesh,
        size_t skeletonIndex,
        const aiScene* scene,
        const aiSkeleton* skeleton)
    {
        KI_INFO_OUT(fmt::format(
            "skeleton={}, name={}, bones={}",
            skeletonIndex,
            skeleton->mName.C_Str(),
            skeleton->mNumBones));

        for (size_t boneIndex = 0; boneIndex < skeleton->mNumBones; ++boneIndex)
        {
            processSkeletonBone(
                modelMesh,
                skeletonIndex,
                boneIndex,
                scene,
                skeleton,
                skeleton->mBones[boneIndex]);
        }
    }

    void AssimpLoader::processSkeletonBone(
        ModelMesh& modelMesh,
        size_t skeletonIndex,
        size_t boneIndex,
        const aiScene* scene,
        const aiSkeleton* skeleton,
        const aiSkeletonBone* bone)
    {
        KI_INFO_OUT(fmt::format(
            "skeleton={}, bone={}, parent={}",
            skeletonIndex, boneIndex, bone->mParent));
    }

    void AssimpLoader::processAnimation(
        ModelMesh& modelMesh,
        size_t animIndex,
        const aiScene* scene,
        const aiAnimation* anim)
    {
        KI_INFO_OUT(fmt::format(
            "anim={}, name={}, duration={}, ticksPerSec={}, channels={}",
            animIndex,
            anim->mName.C_Str(),
            anim->mDuration,
            anim->mTicksPerSecond,
            anim->mNumChannels));

        for (size_t channelIdx = 0; channelIdx < anim->mNumChannels; ++channelIdx)
        {
            const aiNodeAnim* channel = anim->mChannels[channelIdx];
            KI_INFO_OUT(fmt::format(
                "anim={}, channel={}, node={}, posKeys={}, rotKeys={}, scalingKeys={}",
                animIndex,
                channelIdx,
                channel->mNodeName.C_Str(),
                channel->mNumPositionKeys,
                channel->mNumRotationKeys,
                channel->mNumScalingKeys));
        }
    }

    void AssimpLoader::processNode(
        ModelMesh& modelMesh,
        const std::map<size_t, ki::material_id>& materialMapping,
        const aiScene* scene,
        const aiNode* node,
        int level)
    {
        const auto transform = toMat4(node->mTransformation);
        KI_INFO_OUT(fmt::format("node: level={}, name={}, children={}, meshes={}, transform={}",
            level,
            node->mName.C_Str(),
            node->mNumChildren,
            node->mNumMeshes,
            transform));

        for (size_t n = 0; n < node->mNumChildren; ++n)
        {
            processNode(modelMesh, materialMapping, scene, node->mChildren[n], level + 1);
        }

        if (node->mNumMeshes > 0) {
            for (size_t meshIndex = 0; meshIndex < node->mNumMeshes; ++meshIndex)
            {
                processMesh(modelMesh, materialMapping,
                    meshIndex,
                    node,
                    scene->mMeshes[node->mMeshes[meshIndex]],
                    level);
            }
        }
    }

    void AssimpLoader::processMesh(
        ModelMesh& modelMesh,
        const std::map<size_t, ki::material_id>& materialMapping,
        size_t meshIndex,
        const aiNode* node,
        const aiMesh* mesh,
        int level)
    {
        auto& vertices = modelMesh.m_vertices;
        const auto vertexOffset = vertices.size();
        vertices.reserve(vertexOffset + mesh->mNumVertices);

        KI_INFO_OUT(fmt::format("{}: level={}, mesh={}, offset={}, vertices={}, faces={}, bones={}",
            meshIndex, level, modelMesh.m_meshName, vertexOffset, mesh->mNumVertices, mesh->mNumFaces, mesh->mNumBones));

        for (size_t vertexIndex = 0; vertexIndex  < mesh->mNumVertices; vertexIndex++) {
            glm::vec2 texCoord;

            if (mesh->HasTextureCoords(0))
            {
                texCoord = toVec2(mesh->mTextureCoords[0][vertexIndex]);
            }

            const auto pos = toVec3(mesh->mVertices[vertexIndex]);
            glm::vec3 normal{ 0.f };
            glm::vec3 tangent{ 0.f };

            if (mesh->mNormals) {
                normal = toVec3(mesh->mNormals[vertexIndex]);
            }
            if (mesh->mTangents) {
                tangent = toVec3(mesh->mTangents[vertexIndex]);
            }

            ki::material_id materialId;
            {
                const auto& it = materialMapping.find(mesh->mMaterialIndex);
                materialId = it != materialMapping.end() ? it->second : 0;
            }

            if (m_forceDefaultMaterial || !materialId) {
                m_defaultMaterial.m_used = true;
                materialId = m_defaultMaterial.m_id;
            }

            //KI_INFO_OUT(fmt::format("offset={}, pos={}", vertexOffset, pos));

            vertices.emplace_back(pos, texCoord, normal, tangent, materialId);
        }

        for (size_t faceIdx = 0; faceIdx < mesh->mNumFaces; faceIdx++) {
            processMeshFace(modelMesh, meshIndex, faceIdx, vertexOffset, mesh, &mesh->mFaces[faceIdx], level);
        }

        for (size_t boneIdx = 0; boneIdx < mesh->mNumBones; boneIdx++) {
            processMeshBone(modelMesh, meshIndex, boneIdx, vertexOffset, mesh, mesh->mBones[boneIdx], level);
        }
    }

    void AssimpLoader::processMeshFace(
        ModelMesh& modelMesh,
        size_t meshIndex,
        size_t faceIndex,
        size_t vertexOffset,
        const aiMesh* mesh,
        const aiFace* face,
        int level)
    {
        Index index{ 0, 0, 0 };
        for (size_t i = 0; i < face->mNumIndices; i++)
        {
            // NOTE KI multi material models are split by *material* into meshes
            // vertices and thus indeces areare per mesh, but they are *combined*
            // back single mesh in load
            // => must apply vertex offset in index buffer to match that
            index[i] = face->mIndices[i] + vertexOffset;
        }
        //KI_INFO_OUT(fmt::format("face={}, offset={}, idx={}", faceIndex, vertexOffset, index));
        modelMesh.m_indeces.push_back({ index });
    }

    void AssimpLoader::processMeshBone(
        ModelMesh& modelMesh,
        size_t meshIndex,
        size_t boneIndex,
        size_t vertexOffset,
        const aiMesh* mesh,
        const aiBone* bone,
        int level)
    {
        KI_INFO_OUT(fmt::format("mesh={}, offset={}, bone={}, name={}",
            meshIndex, vertexOffset, boneIndex, bone->mName.C_Str()));

        Index index{ 0, 0, 0 };
        for (size_t i = 0; i < bone->mNumWeights; i++)
        {
            const auto* weight =& bone->mWeights[i];
            const auto mat = toMat4(bone->mOffsetMatrix);

            KI_INFO_OUT(fmt::format(
                "mesh={}, bone={}, vertex={}, weight={}, mat={}",
                meshIndex, boneIndex,
                weight->mVertexId, weight->mWeight, mat));
        }
    }

    void AssimpLoader::processMaterials(
        ModelMesh& modelMesh,
        std::map<size_t, ki::material_id>& materialMapping,
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

        result.kd = toVec4(diffuse);
        result.ks = toVec4(specular);
        result.ka = toVec4(ambient);
        result.ke = toVec4(emission);

        result.ns = shininess;

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

        KI_INFO_OUT(fmt::format("ASSIMP: texPath={}", assetPath));

        return assetPath;
    }
}
