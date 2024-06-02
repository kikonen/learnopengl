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

        meshSet.m_rig = std::make_unique<animation::RigContainer>();
        mesh::LoadContext ctx{ meshSet.m_rig.get() };

        processMaterials(meshSet, ctx.m_materials, ctx.m_materialMapping, scene);

        std::vector<const aiNode*> assimpNodes;
        collectNodes(ctx, assimpNodes, scene, scene->mRootNode, -1, glm::mat4{ 1.f });
        //rig.calculateInvTransforms();

        processMeshes(
            ctx,
            meshSet,
            assimpNodes,
            scene);

        loadAnimations(ctx, meshSet.m_name, scene);

        meshSet.m_rig->validate();
    }

    void AssimpLoader::collectNodes(
        mesh::LoadContext& ctx,
        std::vector<const aiNode*>& assimpNodes,
        const aiScene* scene,
        const aiNode* node,
        int16_t parentIndex,
        const glm::mat4& parentTransform)
    {
        auto& rig = *ctx.m_rig;

        uint16_t nodeIndex;
        {
            assimpNodes.push_back(node);

            auto& rigNode = rig.addNode(node);
            rigNode.m_parentIndex = parentIndex;
            nodeIndex = rigNode.m_index;

            KI_INFO_OUT(fmt::format("ASSIMP: NODE node={}, parent={}, name={}, children={}, meshes={}\nT: {}",
                parentIndex,
                nodeIndex,
                node->mName.C_Str(),
                node->mNumChildren,
                node->mNumMeshes,
                rigNode.m_localTransform));
        }

        for (size_t n = 0; n < node->mNumChildren; ++n)
        {
            collectNodes(ctx, assimpNodes, scene, node->mChildren[n], nodeIndex, glm::mat4{ 1.f });
        }
    }

    void AssimpLoader::loadAnimations(
        mesh::LoadContext& ctx,
        const std::string& namePrefix,
        const aiScene* scene)
    {
        animation::AnimationLoader loader{};

        loader.loadAnimations(
            *ctx.m_rig,
            namePrefix,
            scene);
    }

    void AssimpLoader::processMeshes(
        mesh::LoadContext& ctx,
        MeshSet& meshSet,
        const std::vector<const aiNode*>& assimpNodes,
        const aiScene* scene)
    {
        auto& rig = *ctx.m_rig;

        std::vector<glm::mat4> globalTransforms;
        globalTransforms.resize(rig.m_nodes.size() + 1);
        globalTransforms[0] = glm::mat4{ 1.f };

        for (auto& rigNode : rig.m_nodes) {
            const glm::mat4& parentTransform = globalTransforms[rigNode.m_parentIndex + 1];
            globalTransforms[rigNode.m_index + 1] = parentTransform * rigNode.m_localTransform;

            auto& node = assimpNodes[rigNode.m_index];
            if (node->mNumMeshes == 0) continue;

            {
                auto from = std::min((unsigned int)0, node->mNumMeshes - 1);
                auto count = std::min((unsigned int)10, node->mNumMeshes - from);
                for (size_t meshIndex = from; meshIndex < count; ++meshIndex)
                {
                    auto* mesh = scene->mMeshes[node->mMeshes[meshIndex]];

                    auto modelMesh = std::make_unique<mesh::ModelMesh>(mesh->mName.C_Str());
                    modelMesh->setBaseTransform(globalTransforms[rigNode.m_index + 1]);
                    modelMesh->m_rig = ctx.m_rig;
                    modelMesh->m_nodeName = rigNode.m_name;

                    processMesh(
                        ctx,
                        rigNode,
                        *modelMesh,
                        meshIndex,
                        mesh);

                    meshSet.addMesh(std::move(modelMesh));
                }
            }
        }
    }

    void AssimpLoader::processMesh(
        mesh::LoadContext& ctx,
        animation::RigNode& rigNode,
        ModelMesh& modelMesh,
        size_t meshIndex,
        const aiMesh* mesh)
    {
        auto& vertices = modelMesh.m_vertices;
        const auto vertexOffset = vertices.size();
        vertices.reserve(vertexOffset + mesh->mNumVertices);

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


        KI_INFO_OUT(fmt::format("ASSIMP: MESH node={}, name={}, offset={}, material={}, vertices={}, faces={}, bones={}",
            rigNode.m_index,
            modelMesh.m_name,
            vertexOffset,
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

            vertices.emplace_back(pos, texCoord, normal, tangent, 0);
        }

        for (size_t faceIdx = 0; faceIdx < mesh->mNumFaces; faceIdx++) {
            processMeshFace(ctx, modelMesh, meshIndex, faceIdx, vertexOffset, mesh, &mesh->mFaces[faceIdx]);
        }

        for (size_t boneIdx = 0; boneIdx < mesh->mNumBones; boneIdx++) {
            processMeshBone(ctx, modelMesh, meshIndex, vertexOffset, mesh, mesh->mBones[boneIdx]);
        }
    }

    void AssimpLoader::processMeshFace(
        mesh::LoadContext& ctx,
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
        mesh::LoadContext& ctx,
        ModelMesh& modelMesh,
        size_t meshIndex,
        size_t vertexOffset,
        const aiMesh* mesh,
        const aiBone* bone)
    {
        auto& rig = *ctx.m_rig;
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


            auto& vertexBones = modelMesh.m_vertexBones;

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

            assert(vertexIndex < modelMesh.m_vertices.size());

            vertexBones.resize(std::max(vertexIndex + 1, vertexBones.size()));
            vertexBones[vertexIndex].addBone(bi.m_index, vw.mWeight);
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
                material.map_kd = findTexturePath(meshSet, diffusePath.C_Str());
            }
        }
        {
            int bumpIndex = 0;
            aiString bumpPath;
            if (src->GetTexture(aiTextureType_HEIGHT, bumpIndex, &bumpPath) == AI_SUCCESS) {
                material.map_bump = findTexturePath(meshSet, bumpPath.C_Str());
            }
        }
        {
            int normalIndex = 0;
            aiString normalPath;
            if (src->GetTexture(aiTextureType_NORMALS, normalIndex, &normalPath) == AI_SUCCESS) {
                material.map_bump = findTexturePath(meshSet, normalPath.C_Str());
            }
        }
        {
            int emissionIndex = 0;
            aiString emissionPath;
            if (src->GetTexture(aiTextureType_EMISSIVE, emissionIndex, &emissionPath) == AI_SUCCESS) {
                material.map_ke = findTexturePath(meshSet, emissionPath.C_Str());
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
