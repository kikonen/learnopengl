#include "AssimpLoader.h"

#include <iostream>
#include <filesystem>
#include <mutex>

#include <glm/gtx/matrix_decompose.hpp>
#include <fmt/format.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "asset/Assets.h"

#include "util/glm_format.h"
#include "util/glm_util.h"
#include "util/Log.h"
#include "util/util.h"
#include "util/file.h"
#include "util/Transform.h"

#include "animation/RigContainer.h"
#include "animation/RigNode.h"
#include "animation/Animation.h"
#include "animation/RigNodeChannel.h"
#include "animation/JointContainer.h"
#include "animation/Joint.h"
#include "animation/MeshInfo.h"
#include "animation/AnimationLoader.h"

#include "mesh/LoadContext.h"
#include "mesh/MeshSet.h"
#include "mesh/ModelMesh.h"

#include "mesh/RigNodeTreeGenerator.h"

#include "util/assimp_util.h"

namespace {
    std::mutex m_lock;
}

namespace mesh
{
    AssimpLoader::AssimpLoader(
        std::shared_ptr<std::atomic_bool> alive,
        bool debug)
        : ModelLoader{ alive },
        m_debug{ debug }
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

        const auto& assets = Assets::get();

        KI_INFO_OUT(fmt::format("ASSIMP: LOAD_FILE mesh_set={}, path={}",
            meshSet.m_name,
            meshSet.m_filePath));

        if (!util::fileExists(meshSet.m_filePath)) {
            throw std::runtime_error{ fmt::format(
                "FILE_NOT_EXIST: mesh_set={}, path={}",
                meshSet.m_name,
                meshSet.m_filePath) };
        }

        Assimp::Importer importer;

        uint32_t flags =
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
            0;

        if (meshSet.m_smoothNormals) {
            flags |= aiProcess_GenSmoothNormals;
        }
        else {
            flags |= aiProcess_GenNormals;
        }

        if (meshSet.m_forceNormals) {
            flags |= aiProcess_ForceGenNormals;
        }

        const aiScene* scene = importer.ReadFile(
            meshSet.m_filePath,
            flags);

        // If the import failed, report it
        if (!scene) {
            KI_ERROR(importer.GetErrorString());
            return;
        }

        if (m_debug) {
            KI_INFO_OUT(fmt::format(
                "ASSIMP: SCENE scene={}, mesh_set={}, meshes={}, anims={}, materials={}, textures={}",
                meshSet.m_name,
                meshSet.m_filePath,
                scene->mNumMeshes,
                scene->mNumAnimations,
                scene->mNumMaterials,
                scene->mNumTextures));
        }

        auto rig = std::make_shared<animation::RigContainer>(meshSet.m_filePath);
        mesh::LoadContext ctx{ rig };

        processMaterials(meshSet, ctx.m_materials, ctx.m_materialMapping, scene);

        std::vector<const aiNode*> assimpNodes;
        collectNodes(ctx, meshSet, assimpNodes, scene, scene->mRootNode, 0, -1, glm::mat4{ 1.f });

        if (false) {
            dumpMetaData(meshSet, rig->m_nodes, assimpNodes);
        }

        processMeshes(
            ctx,
            meshSet,
            assimpNodes,
            scene);

        if (!rig->empty()) {
            rig->prepare();

            loadAnimations(ctx, "master", meshSet.m_filePath, scene);

            if (assets.animationNodeTree)
            {
                auto* primaryMesh = meshSet.getMesh<mesh::VaoMesh>(0);
                const auto& node = rig->m_nodes[primaryMesh->m_rigNodeIndex];

                util::Transform offset{};
                offset.m_position = glm::vec3{ 20, 0, 0 };

                //const auto offset = glm::translate(glm::mat4{ 1.f }, glm::vec3{ 20, 0, 0 }) *
                //    rigNode.m_globalTransform;

                RigNodeTreeGenerator generator;
                if (auto mesh = generator.generateTree(rig)) {
                    mesh->m_offset = offset;
                    mesh->m_rigNodeIndex = primaryMesh->m_rigNodeIndex;
                    mesh->m_rigNodeName = primaryMesh->m_rigNodeName;
                    meshSet.addMesh(std::move(mesh));
                }

                if (auto mesh = generator.generatePoints(rig)) {
                    mesh->m_offset = offset;
                    mesh->m_rigNodeIndex = primaryMesh->m_rigNodeIndex;
                    mesh->m_rigNodeName = primaryMesh->m_rigNodeName;
                    meshSet.addMesh(std::move(mesh));
                }
            }

            meshSet.m_rig = rig;
        }
        else {
            for (auto& mesh : meshSet.getMeshes()) {
                auto* modelMesh = dynamic_cast<mesh::ModelMesh*>(mesh.get());
                modelMesh->m_rig.reset();

                const auto& rigNode = rig->m_nodes[modelMesh->m_rigNodeIndex];
                const auto& transform = rigNode.m_globalTransform;

                glm::quat rotation;
                glm::vec3 translation;
                glm::vec3 scale;
                glm::vec3 skew;
                glm::vec4 perspective;

                // https://stackoverflow.com/questions/17918033/glm-decompose-mat4-into-translation-and-rotation
                glm::decompose(transform, scale, rotation, translation, skew, perspective);
                //util::decomposeMtx(transform, translation, rotation);

                modelMesh->m_offset.m_position = translation;
                modelMesh->m_offset.m_rotation = rotation;
                modelMesh->m_offset.m_scale = scale;
            }
        }

        if (m_debug) {
            for (auto& mesh : meshSet.m_meshes) {
                KI_INFO_OUT(fmt::format("MESH: mesh_set={}, mesh={}",
                    meshSet.m_name,
                    mesh->str()));
            }
        }
    }

    void AssimpLoader::collectNodes(
        mesh::LoadContext& ctx,
        MeshSet& meshSet,
        std::vector<const aiNode*>& assimpNodes,
        const aiScene* scene,
        const aiNode* node,
        int16_t level,
        int16_t parentIndex,
        const glm::mat4& parentTransform)
    {
        auto& rig = *ctx.m_rig;

        int16_t nodeIndex;
        glm::mat4 globalTransform;
        {
            assimpNodes.push_back(node);

            //glm::mat4 parentTransform = parentTransform2;
            //if (std::string{ "root" } == std::string{ node->mName.C_Str() }) {
            //    //parentTransform = glm::mat4{ 1.f };
            //    parentInvTransform = glm::mat4{ 1.f };
            //}

            auto& rigNode = rig.addNode(node);
            rigNode.m_level = level;
            rigNode.m_parentIndex = parentIndex;
            nodeIndex = rigNode.m_index;

            globalTransform = parentTransform * rigNode.m_transform;
            rigNode.m_globalTransform = globalTransform;
            rigNode.m_globalInvTransform = glm::inverse(globalTransform);

            if (m_debug) {
                KI_INFO_OUT(fmt::format(
                    "ASSIMP: NODE mesh_set={}, node={}.{}, name={}, children={}, meshes={}\nTRAN: {}\nGLOB: {}\nINVE: {}",
                    meshSet.m_name,
                    parentIndex,
                    nodeIndex,
                    node->mName.C_Str(),
                    node->mNumChildren,
                    node->mNumMeshes,
                    rigNode.m_transform,
                    rigNode.m_globalTransform,
                    rigNode.m_globalInvTransform));
            }
        }

        for (size_t n = 0; n < node->mNumChildren; ++n)
        {
            collectNodes(ctx, meshSet, assimpNodes, scene, node->mChildren[n], level + 1, nodeIndex, globalTransform);
        }
    }

    void AssimpLoader::dumpMetaData(
        MeshSet& meshSet,
        const std::vector<animation::RigNode>& nodes,
        const std::vector<const aiNode*>& assimpNodes)
    {
        for (int i = 0; i < nodes.size(); i++) {
            dumpMetaData(meshSet, nodes[i], assimpNodes[i]);
        }
    }

    void AssimpLoader::dumpMetaData(
        MeshSet& meshSet,
        const animation::RigNode& RigNode,
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

            if (m_debug) {
                KI_INFO_OUT(fmt::format(
                    "ASSIMP: META mesh_set={}, node={}.{}, name={}, key={}, value={}",
                    meshSet.m_name,
                    RigNode.m_parentIndex,
                    RigNode.m_index,
                    RigNode.m_name,
                    formattedKey,
                    formattedValue));
            }
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

        for (auto& rigNode : rig.m_nodes) {
            auto& node = assimpNodes[rigNode.m_index];
            if (node->mNumMeshes == 0) continue;

            {
                auto from = std::min((unsigned int)0, node->mNumMeshes - 1);
                auto count = std::min((unsigned int)10, node->mNumMeshes - from);
                for (size_t meshIndex = from; meshIndex < count; ++meshIndex)
                {
                    auto* mesh = scene->mMeshes[node->mMeshes[meshIndex]];
                    const auto* meshName = node->mName.C_Str();
                    const auto* aliasName = mesh->mName.C_Str();

                    auto modelMesh = std::make_unique<mesh::ModelMesh>(meshName);
                    modelMesh->m_alias = aliasName;
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
                        meshSet,
                        rigNode,
                        *modelMesh,
                        meshIndex,
                        mesh);

                    modelMesh->m_rig = ctx.m_rig;

                    modelMesh->m_rigNodeIndex = rigNode.m_index;
                    modelMesh->m_rigNodeName = rigNode.m_name;
                    rigNode.m_mesh = true;

                    // NOTE KI for debug
                    rig.registerMesh(rigNode.m_index, modelMesh.get());

                    meshSet.addMesh(std::move(modelMesh));
                }
            }
        }
    }

    void AssimpLoader::processMesh(
        mesh::LoadContext& ctx,
        MeshSet& meshSet,
        animation::RigNode& RigNode,
        ModelMesh& modelMesh,
        size_t meshIndex,
        const aiMesh* mesh)
    {
        modelMesh.m_vertices.reserve(mesh->mNumVertices);
        modelMesh.m_indeces.reserve(mesh->mNumVertices);

        Material* material{ nullptr };

        {
            const auto& it = ctx.m_materialMapping.find(mesh->mMaterialIndex);
            if (it != ctx.m_materialMapping.end()) {
                material = &ctx.m_materials[it->second];
            } else {
                material = &m_defaultMaterial;
            }
            modelMesh.setMaterial(material);

            //modelMesh.setMaterial(Material::createMaterial(BasicMaterial::blue));
        }

        if (m_debug) {
            KI_INFO_OUT(fmt::format("ASSIMP: MESH mesh_set={}, node={}.{}, name={}, material={}, vertices={}, faces={}, bones={}",
                meshSet.m_name,
                RigNode.m_parentIndex,
                RigNode.m_index,
                modelMesh.m_name,
                material ? material->m_name : fmt::format("{}", mesh->mMaterialIndex),
                mesh->mNumVertices,
                mesh->mNumFaces,
                mesh->mNumBones));
        }

        for (size_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++) {
            glm::vec2 texCoord{ 0.f };

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
            processMeshBone(ctx, meshSet, modelMesh, meshIndex, mesh, mesh->mBones[boneIdx]);
        }

        modelMesh.setupVertexCounts();
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
        MeshSet& meshSet,
        ModelMesh& modelMesh,
        size_t meshIndex,
        const aiMesh* mesh,
        const aiBone* bone)
    {
        auto& joint = ctx.m_rig->registerJoint(bone);

        if (m_debug) {
            KI_INFO_OUT(fmt::format(
                "ASSIMP: BONE mesh_set={}, node={}, joint={}, name={}, mesh={}, weights={}",
                meshSet.m_name,
                joint.m_nodeIndex,
                joint.m_index,
                joint.m_nodeName,
                meshIndex,
                bone->mNumWeights))
        }

        auto& vertexJoints = modelMesh.m_vertexJoints;

        for (size_t i = 0; i < bone->mNumWeights; i++)
        {
            const auto& vw = bone->mWeights[i];
            //const auto mat = assimp_util::toMat4(bone->mOffsetMatrix);

            size_t vertexIndex = bone->mWeights[i].mVertexId;

            assert(vertexIndex < modelMesh.m_vertices.size());

            vertexJoints.resize(std::max(vertexIndex + 1, vertexJoints.size()));
            auto& vb = vertexJoints[vertexIndex];
            vb.addJoint(joint.m_index, vw.mWeight);

            if (m_debug) {
                //KI_INFO_OUT(fmt::format(
            //    "ASSIMP: BONE mesh_set={}, mesh={}, bone={}, vertex={}, vertexJoints={}, vertexWeights={}",
            //    meshSet.m_name,
            //    meshIndex,
            //    joint.m_index,
            //    vertexIndex,
            //    vb.m_boneIds,
            //    vb.m_weights));
            }
        }
    }

    void AssimpLoader::processMaterials(
        const MeshSet& meshSet,
        std::vector<Material>& materials,
        std::map<size_t, size_t>& materialMapping,
        const aiScene* scene)
    {
        for (size_t n = 0; n < scene->mNumMaterials; ++n)
        {
            auto material = processMaterial(meshSet, scene, scene->mMaterials[n]);
            materials.push_back(material);
            materialMapping.insert({ n, materials.size() - 1});
        }
    }

    Material AssimpLoader::processMaterial(
        const MeshSet& meshSet,
        const aiScene* scene,
        const aiMaterial* src)
    {
        const auto name = const_cast<aiMaterial*>(src)->GetName().C_Str();

        if (m_debug) {
            KI_INFO_OUT(fmt::format("ASSIMP: MATERIAL mesh_set={}, name={}, properties={}, allocated={}",
                meshSet.m_name,
                name,
                src->mNumProperties,
                src->mNumAllocated));
        }

        Material material;
        material.m_modelDir = meshSet.m_dir;
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
            //aiColor4D specular;
            //if (aiGetMaterialColor(src, AI_MATKEY_COLOR_SPECULAR, &specular) == AI_SUCCESS) {
            //    material.ks = assimp_util::toVec4(specular);
            //}
        }

        {
            //aiColor4D ambient;
            //if (aiGetMaterialColor(src, AI_MATKEY_COLOR_AMBIENT, &ambient) == AI_SUCCESS) {
            //    material.ka = assimp_util::toVec4(ambient);
            //}
        }
        {
            aiColor4D emission;
            if (aiGetMaterialColor(src, AI_MATKEY_COLOR_EMISSIVE, &emission) == AI_SUCCESS) {
                material.ke = assimp_util::toVec4(emission);
            }
        }

        {
            //ai_real shininess;
            //if (aiGetMaterialFloat(src, AI_MATKEY_SHININESS, &shininess) == AI_SUCCESS) {
            //    material.ns = shininess;
            //}

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
                material.addTexture(
                    TextureType::diffuse,
                    findTexturePath(meshSet, diffusePath.C_Str()),
                    true);
            }
        }
        {
            int bumpIndex = 0;
            aiString bumpPath;
            if (src->GetTexture(aiTextureType_HEIGHT, bumpIndex, &bumpPath) == AI_SUCCESS) {
                material.addTexture(
                    TextureType::map_normal,
                    findTexturePath(meshSet, bumpPath.C_Str()),
                    true);
            }
        }
        {
            int normalIndex = 0;
            aiString normalPath;
            if (src->GetTexture(aiTextureType_NORMALS, normalIndex, &normalPath) == AI_SUCCESS) {
                material.addTexture(
                    TextureType::map_normal,
                    findTexturePath(meshSet, normalPath.C_Str()),
                    true);
            }
        }
        {
            int emissionIndex = 0;
            aiString emissionPath;
            if (src->GetTexture(aiTextureType_EMISSIVE, emissionIndex, &emissionPath) == AI_SUCCESS) {
                material.addTexture(
                    TextureType::emission,
                    findTexturePath(meshSet, emissionPath.C_Str()),
                    true);
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
        //std::string assetPath2 = std::filesystem::weakly_canonical(fsPath).string();

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

        if (m_debug) {
            KI_INFO_OUT(fmt::format("ASSIMP: TEX mesh_set={}, path={}, was={}",
                meshSet.m_name,
                assetPath,
                origPath));
        }

        return assetPath;
    }
}
