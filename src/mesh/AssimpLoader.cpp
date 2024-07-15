#include "AssimpLoader.h"

#include <iostream>
#include <filesystem>
#include <mutex>

#include <fmt/format.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "util/glm_format.h"
#include "util/Log.h"
#include "util/Util.h"

#include "animation/RigContainer.h"
#include "animation/RigJoint.h"
#include "animation/Animation.h"
#include "animation/BoneChannel.h"
#include "animation/BoneContainer.h"
#include "animation/BoneInfo.h"
#include "animation/MeshInfo.h"
#include "animation/AnimationLoader.h"

#include "mesh/LoadContext.h"
#include "mesh/MeshSet.h"
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
        mesh::MeshSet& meshSet)
    {
        std::string filePath = assimp_util::resolvePath(
            meshSet.m_rootDir,
            meshSet.m_path);

        if (filePath.empty()) return;

        meshSet.m_filePath = filePath;
        loadResolvedPath(meshSet);
    }

    void AssimpLoader::loadResolvedPath(
        mesh::MeshSet& meshSet)
    {
        std::lock_guard lock(m_lock);

        KI_INFO_OUT(fmt::format("ASSIMP: FILE path={}", meshSet.m_filePath));

        if (!util::fileExists(meshSet.m_filePath)) {
            throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", meshSet.m_filePath) };
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(
            meshSet.m_filePath,
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
            //aiProcess_ValidateDataStructure |
            0);

        // If the import failed, report it
        if (!scene) {
            KI_ERROR(importer.GetErrorString());
            return;
        }

        KI_INFO_OUT(fmt::format(
            "ASSIMP: SCENE scene={}, meshes={}, anims={}, materials={}, textures={}",
            meshSet.m_filePath,
            scene->mNumMeshes,
            scene->mNumAnimations,
            scene->mNumMaterials,
            scene->mNumTextures));

        auto rig = std::make_shared<animation::RigContainer>(meshSet.m_filePath);
        mesh::LoadContext ctx{ rig };

        processMaterials(meshSet, ctx.m_materials, ctx.m_materialMapping, scene);

        std::vector<const aiNode*> assimpNodes;
        collectJoints(ctx, assimpNodes, scene, scene->mRootNode, 0, -1, glm::mat4{ 1.f }, glm::mat4{ 1.f });
        dumpMetaData(rig->m_joints, assimpNodes);

        processMeshes(
            ctx,
            meshSet,
            assimpNodes,
            scene);

        if (!rig->empty()) {
            loadAnimations(ctx, meshSet.m_name, meshSet.m_filePath, scene);

            rig->prepare();
            meshSet.m_rig = rig;
        }
        else {
            for (auto& mesh : meshSet.getMeshes()) {
                auto* modelMesh = dynamic_cast<mesh::ModelMesh*>(mesh.get());
                modelMesh->m_rig.reset();
            }
        }

        for (auto& mesh : meshSet.m_meshes) {
            KI_INFO_OUT(fmt::format("MESH: {}", mesh->str()));
        }
    }

    void AssimpLoader::collectJoints(
        mesh::LoadContext& ctx,
        std::vector<const aiNode*>& assimpNodes,
        const aiScene* scene,
        const aiNode* node,
        int16_t level,
        int16_t parentIndex,
        const glm::mat4& parentTransform2,
        const glm::mat4& parentInvTransform2)
    {
        auto& rig = *ctx.m_rig;

        int16_t jointIndex;
        glm::mat4 globalTransform;
        glm::mat4 globalInvTransform;
        {
            assimpNodes.push_back(node);

            glm::mat4 parentTransform = parentTransform2;
            glm::mat4 parentInvTransform = parentInvTransform2;

            //if (std::string{ "root" } == std::string{ node->mName.C_Str() }) {
            //    //parentTransform = glm::mat4{ 1.f };
            //    parentInvTransform = glm::mat4{ 1.f };
            //}

            auto& rigJoint = rig.addJoint(node);
            rigJoint.m_level = level;
            rigJoint.m_parentIndex = parentIndex;
            jointIndex = rigJoint.m_index;

            globalTransform = parentTransform * rigJoint.m_transform;
            globalInvTransform = parentInvTransform * rigJoint.m_invTransform;
            rigJoint.m_globalTransform = globalTransform;
            rigJoint.m_globalInvTransform = globalInvTransform;

            KI_INFO_OUT(fmt::format(
                "ASSIMP: NODE node={}.{}, name={}, children={}, meshes={}\nT: {}\nG: {}\nI: {}",
                parentIndex,
                jointIndex,
                node->mName.C_Str(),
                node->mNumChildren,
                node->mNumMeshes,
                rigJoint.m_transform,
                rigJoint.m_globalTransform,
                rigJoint.m_globalInvTransform));
        }

        for (size_t n = 0; n < node->mNumChildren; ++n)
        {
            collectJoints(ctx, assimpNodes, scene, node->mChildren[n], level + 1, jointIndex, globalTransform, globalInvTransform);
        }
    }

    void AssimpLoader::dumpMetaData(
        const std::vector<animation::RigJoint>& joints,
        const std::vector<const aiNode*>& assimpNodes)
    {
        for (int i = 0; i < joints.size(); i++) {
            dumpMetaData(joints[i], assimpNodes[i]);
        }
    }

    void AssimpLoader::dumpMetaData(
        const animation::RigJoint& rigJoint,
        const aiNode* node)
    {
        const std::string nodeName{ node->mName.C_Str() };
        const aiMetadata* meta = node->mMetaData;
        if (!meta) return;

        for (size_t i = 0; i < meta->mNumProperties; i++) {
            const auto& key = meta->mKeys[i];
            const auto& value = meta->mValues[i];

            std::string formattedKey{ key.C_Str() };
            std::string formattedValue;

            bool boolValue;
            int32_t intValue;
            //uint32_t uintValue;
            //int64_t longValue;
            //uint64_t ulongValue;
            float floatValue;
            double doubleValue;
            aiString stringValue;

            switch (value.mType) {
            case AI_BOOL:
                meta->Get(key, boolValue);
                formattedValue = fmt::format("{}", boolValue);
                break;
            case AI_INT32:
                meta->Get(key, intValue);
                formattedValue = fmt::format("{}", intValue);
                break;
            //case AI_UINT64:
            //    meta->Get(key, ulongValue);
            //    formattedValue = fmt::format("{}", ulongValue);
            //    break;
            case AI_FLOAT:
                meta->Get(key, floatValue);
                formattedValue = fmt::format("{}", floatValue);
                break;
            case AI_DOUBLE:
                meta->Get(key, doubleValue);
                formattedValue = fmt::format("{}", doubleValue);
                break;
            case AI_AISTRING:
                meta->Get(key, stringValue);
                formattedValue = fmt::format("{}", stringValue.C_Str());
                break;
            case AI_AIVECTOR3D:
                break;
            //case AI_AIMETADATA:
            //    break;
            //case AI_INT64:
            //    meta->Get(key, longValue);
            //    formattedValue = fmt::format("{}", longValue);
            //    break;
            //case AI_UINT32:
            //    meta->Get(key, uintValue);
            //    formattedValue = fmt::format("{}", uintValue);
            //    break;
            default:
                formattedValue = "<unknown>";
                break;
            }

            KI_INFO_OUT(fmt::format(
                "ASSIMP: META node={}.{}, name={}, key={}, value={}",
                rigJoint.m_parentIndex,
                rigJoint.m_index,
                rigJoint.m_name,
                formattedKey,
                formattedValue));
        }
    }

    void AssimpLoader::loadAnimations(
        mesh::LoadContext& ctx,
        const std::string& namePrefix,
        const std::string& filePath,
        const aiScene* scene)
    {
        animation::AnimationLoader loader{};

        loader.loadAnimations(
            *ctx.m_rig,
            namePrefix,
            filePath,
            scene);
    }

    void AssimpLoader::processMeshes(
        mesh::LoadContext& ctx,
        MeshSet& meshSet,
        const std::vector<const aiNode*>& assimpNodes,
        const aiScene* scene)
    {
        auto& rig = *ctx.m_rig;

        for (auto& rigJoint : rig.m_joints) {
            auto& node = assimpNodes[rigJoint.m_index];
            if (node->mNumMeshes == 0) continue;

            {
                auto from = std::min((unsigned int)0, node->mNumMeshes - 1);
                auto count = std::min((unsigned int)10, node->mNumMeshes - from);
                for (size_t meshIndex = from; meshIndex < count; ++meshIndex)
                {
                    auto* mesh = scene->mMeshes[node->mMeshes[meshIndex]];

                    auto modelMesh = std::make_unique<mesh::ModelMesh>(mesh->mName.C_Str());
                    //if (modelMesh->m_name == std::string{ "SK_Armor" }) continue;
                    //if (modelMesh->m_name == std::string{ "SM_Helmet" }) continue;
                    //if (modelMesh->m_name == std::string{ "SM_2HandedSword" }) continue;
                    //if (modelMesh->m_name == std::string{ "WEAPON_BONE" }) continue;
                    //if (modelMesh->m_name == std::string{ "SM_Sword" }) continue;
                    //if (modelMesh->m_name == std::string{ "SM_Shield" }) continue;
                    //if (modelMesh->m_name == std::string{ "skeleton_knight" }) continue;

                    //if (modelMesh->m_name == std::string{ "UBX_SM_FieldFences01a_LOD0_data.003" }) continue;
                    //if (modelMesh->m_name == std::string{ "UBX_SM_FieldFences01a_LOD0_data.004" }) continue;
                    //if (modelMesh->m_name == std::string{ "UBX_SM_FieldFences01a_LOD0_data.005" }) continue;

                    processMesh(
                        ctx,
                        rigJoint,
                        *modelMesh,
                        meshIndex,
                        mesh);

                    modelMesh->m_rig = ctx.m_rig;

                    modelMesh->m_rigJointName = rigJoint.m_name;

                    // NOTE KI for animated meshes, this transform is canceled in animator
                    modelMesh->setRigTransform(rigJoint.m_globalTransform);

                    // NOTE KI for debug
                    rig.registerMesh(rigJoint.m_index, modelMesh.get());

                    meshSet.addMesh(std::move(modelMesh));
                }
            }
        }
    }

    void AssimpLoader::processMesh(
        mesh::LoadContext& ctx,
        animation::RigJoint& rigJoint,
        ModelMesh& modelMesh,
        size_t meshIndex,
        const aiMesh* mesh)
    {
        modelMesh.m_vertices.reserve(mesh->mNumVertices);
        modelMesh.m_indeces.reserve(mesh->mNumVertices);

        Material* material{ nullptr };

        {
            const auto& it = ctx.m_materialMapping.find(mesh->mMaterialIndex);
            auto materialId = it != ctx.m_materialMapping.end() ? it->second : 0;
            material = Material::findID(materialId, ctx.m_materials);
            if (!material) {
                material = &m_defaultMaterial;
            }
            modelMesh.setMaterial(*material);
        }


        KI_INFO_OUT(fmt::format("ASSIMP: MESH node={}.{}, name={}, material={}, vertices={}, faces={}, bones={}",
            rigJoint.m_parentIndex,
            rigJoint.m_index,
            modelMesh.m_name,
            material ? material->m_name : fmt::format("{}", mesh->mMaterialIndex),
            mesh->mNumVertices,
            mesh->mNumFaces,
            mesh->mNumBones));

        for (size_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++) {
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

            modelMesh.m_vertices.emplace_back(pos, texCoord, normal, tangent);
        }

        for (size_t faceIdx = 0; faceIdx < mesh->mNumFaces; faceIdx++) {
            processMeshFace(ctx, modelMesh, meshIndex, faceIdx, mesh, &mesh->mFaces[faceIdx]);
        }

        for (size_t boneIdx = 0; boneIdx < mesh->mNumBones; boneIdx++) {
            processMeshBone(ctx, modelMesh, meshIndex, mesh, mesh->mBones[boneIdx]);
        }
    }

    void AssimpLoader::processMeshFace(
        mesh::LoadContext& ctx,
        ModelMesh& modelMesh,
        size_t meshIndex,
        size_t faceIndex,
        const aiMesh* mesh,
        const aiFace* face)
    {
        assert(face->mNumIndices <= 3);

        for (uint32_t i = 0; i < face->mNumIndices; i++)
        {
            modelMesh.m_indeces.push_back(face->mIndices[i]);
        }
    }

    void AssimpLoader::processMeshBone(
        mesh::LoadContext& ctx,
        ModelMesh& modelMesh,
        size_t meshIndex,
        const aiMesh* mesh,
        const aiBone* bone)
    {
        auto& bi = ctx.m_rig->registerBone(bone);

        KI_INFO_OUT(fmt::format(
            "ASSIMP: BONE joint={}, bone={}, name={}, mesh={}, weights={}",
            bi.m_jointIndex,
            bi.m_index,
            bi.m_jointName,
            meshIndex,
            bone->mNumWeights))

        auto& vertexBones = modelMesh.m_vertexBones;

        for (size_t i = 0; i < bone->mNumWeights; i++)
        {
            const auto& vw = bone->mWeights[i];
            const auto mat = assimp_util::toMat4(bone->mOffsetMatrix);

            size_t vertexIndex = bone->mWeights[i].mVertexId;

            assert(vertexIndex < modelMesh.m_vertices.size());

            vertexBones.resize(std::max(vertexIndex + 1, vertexBones.size()));
            auto& vb = vertexBones[vertexIndex];
            vb.addBone(bi.m_index, vw.mWeight);

            //KI_INFO_OUT(fmt::format(
            //    "ASSIMP: mesh={}, bone={}, vertex={}, vertexBones={}, vertexWeights={}",
            //    meshIndex,
            //    bi.m_index,
            //    vertexIndex,
            //    vb.m_boneIds,
            //    vb.m_weights));
        }
    }

    void AssimpLoader::processMaterials(
        const MeshSet& meshSet,
        std::vector<Material>& materials,
        std::map<size_t, ki::material_id>& materialMapping,
        const aiScene* scene)
    {
        for (size_t n = 0; n < scene->mNumMaterials; ++n)
        {
            auto material = processMaterial(meshSet, scene, scene->mMaterials[n]);
            materials.push_back(material);
            materialMapping.insert({ n, material.m_id });
        }
    }

    Material AssimpLoader::processMaterial(
        const MeshSet& meshSet,
        const aiScene* scene,
        const aiMaterial* src)
    {
        const auto name = const_cast<aiMaterial*>(src)->GetName().C_Str();

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
                material.addTexPath(
                    TextureType::diffuse,
                    findTexturePath(meshSet, diffusePath.C_Str()));
            }
        }
        {
            int bumpIndex = 0;
            aiString bumpPath;
            if (src->GetTexture(aiTextureType_HEIGHT, bumpIndex, &bumpPath) == AI_SUCCESS) {
                material.addTexPath(
                    TextureType::normal_map,
                    findTexturePath(meshSet, bumpPath.C_Str()));
            }
        }
        {
            int normalIndex = 0;
            aiString normalPath;
            if (src->GetTexture(aiTextureType_NORMALS, normalIndex, &normalPath) == AI_SUCCESS) {
                material.addTexPath(
                    TextureType::normal_map,
                    findTexturePath(meshSet, normalPath.C_Str()));
            }
        }
        {
            int emissionIndex = 0;
            aiString emissionPath;
            if (src->GetTexture(aiTextureType_EMISSIVE, emissionIndex, &emissionPath) == AI_SUCCESS) {
                material.addTexPath(
                    TextureType::emission,
                    findTexturePath(meshSet, emissionPath.C_Str()));
            }
        }

        return material;
    }

    std::string AssimpLoader::findTexturePath(
        const MeshSet& meshSet,
        const std::string& origPath)
    {
        const auto& rootDir = meshSet.m_rootDir;
        const auto& meshName = meshSet.m_name;

        std::string assetPath = origPath;
        std::filesystem::path meshPath{ meshName };
        const auto parentPath = meshPath.parent_path();

        std::filesystem::path fsPath{ assetPath };
        std::string assetPath2 = std::filesystem::weakly_canonical(fsPath).string();

        std::string filePath = util::joinPathExt(
            rootDir,
            parentPath.string(),
            assetPath, "");

        if (util::fileExists(filePath)) {
            //assetPath = util::joinPath(
            //    parentPath.filename().string(),
            //    assetPath);
            assetPath = filePath.substr(
                rootDir.length() + 1,
                filePath.length() - rootDir.length() - 1);
        }

        KI_INFO_OUT(fmt::format("ASSIMP: TEX path={}, was={}", assetPath, origPath));

        return assetPath;
    }
}
