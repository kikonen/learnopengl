#include "AssimpLoader.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <mutex>
#include <set>

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

#include "mesh/MeshSet.h"
#include "mesh/ModelMesh.h"

#include "util/assimp_util.h"

#include "AssimpLogger.h"
#include "AnimationLoader.h"
#include "AssimpMaterialLoader.h"
#include "LoadContext.h"
#include "RigNodeTreeGenerator.h"

namespace
{
    std::mutex m_lock;

    const char EMBEDDED_NAME_PREFIX = '*';

    bool isEmbeddedTexture(const std::string name)
    {
        return name[0] == EMBEDDED_NAME_PREFIX;
    }
}

namespace mesh_set
{
    AssimpLoader::AssimpLoader(
        const std::shared_ptr<std::atomic_bool>& alive,
        bool debug)
        : MeshSetLoader{ alive },
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

        AssimpLogger::attach();

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
        LoadContext ctx{ rig };

        AssimpMaterialLoader materialLoader{ m_debug };
        materialLoader.processMaterials(meshSet, ctx.m_materials, ctx.m_materialMapping, scene);

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
        LoadContext& ctx,
        mesh::MeshSet& meshSet,
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

            if (m_debug) {
                KI_INFO_OUT(fmt::format(
                    "ASSIMP: NODE mesh_set={}, node={}.{}, name={}, children={}, meshes={}\nTRAN: {}\nGLOB: {}",
                    meshSet.m_name,
                    parentIndex,
                    nodeIndex,
                    node->mName.C_Str(),
                    node->mNumChildren,
                    node->mNumMeshes,
                    rigNode.m_transform,
                    rigNode.m_globalTransform));
            }
        }

        for (size_t n = 0; n < node->mNumChildren; ++n)
        {
            collectNodes(ctx, meshSet, assimpNodes, scene, node->mChildren[n], level + 1, nodeIndex, globalTransform);
        }
    }

    void AssimpLoader::dumpMetaData(
        mesh::MeshSet& meshSet,
        const std::vector<animation::RigNode>& nodes,
        const std::vector<const aiNode*>& assimpNodes)
    {
        for (int i = 0; i < nodes.size(); i++) {
            dumpMetaData(meshSet, nodes[i], assimpNodes[i]);
        }
    }

    void AssimpLoader::dumpMetaData(
        mesh::MeshSet& meshSet,
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
        LoadContext& ctx,
        const std::string& namePrefix,
        const std::string& filePath,
        const aiScene* scene)
    {
        AnimationLoader loader{};

        loader.loadAnimations(
            *ctx.m_rig,
            namePrefix,
            filePath,
            scene);
    }

    void AssimpLoader::processMeshes(
        LoadContext& ctx,
        mesh::MeshSet& meshSet,
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
        LoadContext& ctx,
        mesh::MeshSet& meshSet,
        animation::RigNode& rigNode,
        mesh::ModelMesh& modelMesh,
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
                rigNode.m_parentIndex,
                rigNode.m_index,
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
        LoadContext& ctx,
        mesh::ModelMesh& modelMesh,
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
        LoadContext& ctx,
        mesh::MeshSet& meshSet,
        mesh::ModelMesh& modelMesh,
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
        vertexJoints.reserve(modelMesh.m_vertices.size());
        vertexJoints.resize(modelMesh.m_vertices.size());

        for (size_t i = 0; i < bone->mNumWeights; i++)
        {
            const auto& vw = bone->mWeights[i];
            //const auto mat = assimp_util::toMat4(bone->mOffsetMatrix);

            size_t vertexIndex = vw.mVertexId;

            assert(vertexIndex < modelMesh.m_vertices.size());

            vertexJoints.resize(std::max(vertexIndex + 1, vertexJoints.size()));
            auto& vb = vertexJoints[vertexIndex];

            float weight = vw.mWeight;
            if (true) {
                if (std::isnan(weight))
                    continue;

                if (weight != 0 && std::fabsf(weight) < std::numeric_limits<float>::min()) {
                    int x = 0;
                    weight = 0.f;
                }
                if (weight < 0) {
                    int x = 0;
                    weight = 0.f;
                }
                if (weight > 1) {
                    int x = 0;
                    weight = 1.f;
                }

                if (!weight)
                    continue;
            }

            vb.addJoint(joint.m_index, weight);

            if (false && m_debug) {
                KI_INFO_OUT(fmt::format(
                    "ASSIMP: JOINT_VERTEX_BIND mesh_set={}, mesh={}, node={}, joint={}, vertex={}, vertexJoints={}, vertexWeights={}",
                    meshSet.m_name,
                    meshIndex,
                    joint.m_nodeIndex,
                    joint.m_index,
                    vertexIndex,
                    vb.m_jointIds,
                    vb.m_weights));
            }
        }
    }
}
