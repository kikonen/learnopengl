#include "AssimpImporter.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <mutex>
#include <set>
#include <ranges>

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
#include "util/util_join.h"
#include "util/assimp_util.h"
#include "util/file.h"
#include "util/Transform.h"

#include "animation/RigNode.h"
#include "animation/Animation.h"
#include "animation/RigNodeChannel.h"
#include "animation/JointContainer.h"
#include "animation/Joint.h"
#include "animation/MeshInfo.h"
#include "animation/Rig.h"

#include "mesh/MeshSet.h"
#include "mesh/ModelMesh.h"

#include "util/assimp_util.h"

#include "AssimpLogger.h"
#include "AnimationImporter.h"
#include "AssimpMaterialImporter.h"
#include "LoadContext.h"
#include "RigNodeTreeGenerator.h"
#include "SkeletonSet.h"

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
    AssimpImporter::AssimpImporter(
        const std::shared_ptr<std::atomic_bool>& alive,
        bool debug)
        : MeshSetImporter{ alive },
        m_debug{ debug }
    {}

    AssimpImporter::~AssimpImporter()
    {}

    void AssimpImporter::loadData(
        mesh::MeshSet& meshSet)
    {
        std::string filePath = assimp_util::resolvePath(
            meshSet.m_rootDir,
            meshSet.m_path);

        if (filePath.empty()) return;

        meshSet.m_filePath = filePath;
        loadResolvedPath(meshSet);
    }

    // FBX Full Transform Model
    // FBX stores transforms as:
    // 
    // WorldTransform = ParentWorldTransform *
    // T* Roff* Rp* Rpre* R* Rpost* Rp⁻¹* Soff* Sp* S* Sp⁻¹
    //
    //     Where :
    // T = Translation
    //     Roff = Rotation offset
    //     Rp = Rotation pivot
    //     Rpre = Pre - rotation(geometric correction)
    //     R = Rotation
    //     Rpost = Post - rotation
    //     Sp = Scaling pivot
    //     Soff = Scaling offset
    //     S = Scaling
    //
    // Bone_$AssimpFbx$_PreRotation    (stores Rpre)
    // └── Bone_$AssimpFbx$_Rotation(stores R)
    //     └── Bone_$AssimpFbx$_PostRotation(stores Rpost)
    //     └── Bone_$AssimpFbx$_Scaling(stores S)
    //     └── Bone_$AssimpFbx$_Translation(stores T)
    //     └── Bone(identity or remaining transform)
    //
    void AssimpImporter::loadResolvedPath(
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
            //aiProcess_GenUVCoords |
            //aiProcess_SortByPType |
            //aiProcess_GlobalScale |
            aiProcess_PopulateArmatureData |
            //aiProcess_GenBoundingBoxes |
            // Validation
            //aiProcess_ValidateDataStructure |
            //aiProcess_FindDegenerates |
            //aiProcess_FindInvalidData |
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

        LoadContext ctx{};

        AssimpMaterialImporter materialImporter{ m_debug };
        materialImporter.processMaterials(meshSet, ctx.m_materials, ctx.m_materialMapping, scene);

        SkeletonSet skeletonSet;
        skeletonSet.resolve(scene);

        processMeshes(
            ctx,
            meshSet,
            skeletonSet,
            scene,
            scene->mRootNode);

        std::set<const animation::Rig*> processedRigs;

        std::vector<std::shared_ptr<mesh::Mesh>> additionalMeshes;

        float xOffset = 0.f;
        float zOffset = 0.f;

        for (auto& mesh : meshSet.getMeshes()) {
            auto* modelMesh = dynamic_cast<mesh::ModelMesh*>(mesh.get());

            if (!modelMesh) continue;

            auto* rig = modelMesh->m_rig.get();
            if (!rig) continue;

            auto& jointContainer = rig->getJointContainer();

            if (!jointContainer.empty())
            {
                if (processedRigs.contains(rig)) continue;
                processedRigs.insert(rig);

                loadAnimations(ctx, *rig, "master", meshSet.m_filePath, scene);

                if (assets.animationNodeTree)
                {
                    auto* primaryMesh = meshSet.getMesh<mesh::VaoMesh>(0);
                    //const auto& rigNode = rig->m_nodes[primaryMesh->m_rigNodeIndex];

                    util::Transform offset{};
                    xOffset += 40.f;
                    zOffset += 40.f;
                    offset.m_position = glm::vec3{ xOffset, 0, zOffset };

                    //const auto offset = glm::translate(glm::mat4{ 1.f }, glm::vec3{ 20, 0, 0 }) *
                    //    rigNode.m_globalTransform;

                    RigNodeTreeGenerator generator;
                    if (auto mesh = generator.generateTree(modelMesh->m_rig)) {
                        mesh->m_offset = offset;
                        mesh->m_rigNodeIndex = primaryMesh->m_rigNodeIndex;
                        additionalMeshes.push_back(std::move(mesh));
                    }

                    if (auto mesh = generator.generatePoints(modelMesh->m_rig)) {
                        mesh->m_offset = offset;
                        mesh->m_rigNodeIndex = primaryMesh->m_rigNodeIndex;
                        additionalMeshes.push_back(std::move(mesh));
                    }
                }
            }
            else {
                modelMesh->m_rig.reset();

                const auto& rigNode = modelMesh->m_rig->m_nodes[modelMesh->m_rigNodeIndex];
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

        for (const auto& mesh : additionalMeshes) {
            meshSet.addMesh(mesh);
        }

        if (m_debug) {
            for (auto& mesh : meshSet.m_meshes) {
                KI_INFO_OUT(fmt::format("MESH: mesh_set={}, mesh={}",
                    meshSet.m_name,
                    mesh->str()));
            }
        }
    }

    void AssimpImporter::loadAnimations(
        LoadContext& ctx,
        animation::Rig& rig,
        const std::string& namePrefix,
        const std::string& filePath,
        const aiScene* scene)
    {
        AnimationImporter importer{};

        importer.loadAnimations(
            rig,
            namePrefix,
            filePath,
            scene);
    }

    void AssimpImporter::processMeshes(
        LoadContext& ctx,
        mesh::MeshSet& meshSet,
        const SkeletonSet& skeletonSet,
        const aiScene* scene,
        const aiNode* node)
    {
        for (unsigned int meshIndex = 0; meshIndex < node->mNumMeshes; meshIndex++)
        {
            auto* mesh = scene->mMeshes[node->mMeshes[meshIndex]];

            const auto& nodeName = assimp_util::normalizeName(node->mName);
            const auto& aliasName = assimp_util::normalizeName(mesh->mName);

            auto modelMesh = std::make_unique<mesh::ModelMesh>(nodeName);
            modelMesh->m_alias = aliasName;

            const auto& rig = skeletonSet.findRig(mesh);

            processMesh(
                ctx,
                meshSet,
                rig.get(),
                *modelMesh,
                node,
                mesh);

            // NOTE KI without rig, no bones or animation
            if (rig && mesh->mNumBones > 0)
            {
                modelMesh->m_rig = rig;

                // NOTE KI for debugging/troubleshooting only
                auto& rootNode = rig->m_nodes[0];
                {
                    modelMesh->m_rigNodeIndex = rootNode.m_index;
                    rootNode.m_hasMesh = true;

                    // NOTE KI for debug
                    rig->registerMesh(rootNode.m_index, modelMesh.get());
                }
            }

            meshSet.addMesh(std::move(modelMesh));
        }

        for (unsigned int childIndex = 0; childIndex < node->mNumChildren; childIndex++) {
            processMeshes(ctx, meshSet, skeletonSet, scene, node->mChildren[childIndex]);
        }
    }

    void AssimpImporter::processMesh(
        LoadContext& ctx,
        mesh::MeshSet& meshSet,
        animation::Rig* rig,
        mesh::ModelMesh& modelMesh,
        const aiNode* node,
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
            KI_INFO_OUT(fmt::format("ASSIMP: MESH mesh_set={}, name={}, material={}, vertices={}, faces={}, bones={}",
                meshSet.m_name,
                modelMesh.m_name,
                material ? material->m_name : fmt::format("{}", mesh->mMaterialIndex),
                mesh->mNumVertices,
                mesh->mNumFaces,
                mesh->mNumBones));
        }

        for (unsigned int vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++) {
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

        for (unsigned int faceIdx = 0; faceIdx < mesh->mNumFaces; faceIdx++) {
            processMeshFace(ctx, modelMesh,faceIdx, mesh, &mesh->mFaces[faceIdx]);
        }

        // NOTE KI without rig, no bones or animation
        if (rig && mesh->mNumBones > 0)
        {
            for (unsigned int boneIdx = 0; boneIdx < mesh->mNumBones; boneIdx++) {
                processMeshBone(
                    *rig,
                    modelMesh,
                    mesh->mBones[boneIdx]);
            }
        }

        modelMesh.setupVertexCounts();
    }

    void AssimpImporter::processMeshFace(
        LoadContext& ctx,
        mesh::ModelMesh& modelMesh,
        size_t faceIndex,
        const aiMesh* mesh,
        const aiFace* face)
    {
        assert(face->mNumIndices <= 3);

        for (unsigned int i = 0; i < face->mNumIndices; i++)
        {
            modelMesh.m_indeces.push_back(face->mIndices[i]);
        }
    }

    void AssimpImporter::processMeshBone(
        animation::Rig& rig,
        mesh::ModelMesh& modelMesh,
        const aiBone* bone)
    {
        const auto* joint = rig.registerJoint(bone);

        if (!joint) {
            return;
        }

        const auto* rigNode = rig.getNode(joint->m_nodeIndex);

        const auto& boneInfo = fmt::format(
            "mesh={}, node={}.{}, joint={}, weights={}",
            modelMesh.m_name,
            joint->m_nodeIndex,
            rigNode ? rigNode->m_name : "NA",
            joint->m_jointIndex,
            bone->mNumWeights);

        if (true || m_debug) {
            KI_INFO_OUT(fmt::format(
                "ASSIMP: BONE {}",
                boneInfo))
        }

        auto& vertexJoints = modelMesh.m_vertexJoints;
        vertexJoints.reserve(modelMesh.m_vertices.size());
        vertexJoints.resize(modelMesh.m_vertices.size());

        for (unsigned int i = 0; i < bone->mNumWeights; i++)
        {
            const auto& vw = bone->mWeights[i];
            //const auto mat = assimp_util::toMat4(bone->mOffsetMatrix);

            unsigned int vertexIndex = vw.mVertexId;

            assert(vertexIndex < modelMesh.m_vertices.size());

            vertexJoints.resize(std::max(vertexIndex + 1, static_cast<unsigned int>(vertexJoints.size())));
            auto& vb = vertexJoints[vertexIndex];

            float weight = vw.mWeight;
            if (true) {
                if (std::isnan(weight)) {
                    KI_ERROR_OUT(fmt::format(
                        "ASSIMP: BONE_NAN_WEIGHT {}, vertex={}, weight_index={}, weight={}",
                        boneInfo,
                        vertexIndex,
                        i,
                        weight));
                    continue;
                }

                if (weight != 0 && std::fabsf(weight) < std::numeric_limits<float>::min()) {
                    //KI_ERROR_OUT(fmt::format(
                    //    "ASSIMP: BONE_DEN_WEIGHT {}, vertex={}, weight_index={}, weight={}",
                    //    boneInfo,
                    //    vertexIndex,
                    //    i,
                    //    weight));
                    int x = 0;
                    weight = 0.f;
                }
                if (weight < 0) {
                    KI_ERROR_OUT(fmt::format(
                        "ASSIMP: BONE_NEGATIVE_WEIGHT {}, vertex={}, weight_index={}, weight={}",
                        boneInfo,
                        vertexIndex,
                        i,
                        weight));
                    int x = 0;
                    weight = 0.0000001f;
                }
                if (weight > 1) {
                    KI_ERROR_OUT(fmt::format(
                        "ASSIMP: BONE_INF_WEIGHT {}, vertex={}, weight_index={}, weight={}",
                        boneInfo,
                        vertexIndex,
                        i,
                        weight));
                    int x = 0;
                    weight = 1.f;
                    //weight = 0.f;
                }

                if (!weight) {
                    //KI_ERROR_OUT(fmt::format(
                    //    "ASSIMP: BONE_SKIP_WEIGHT {}, vertex={}, weight_index={}, weight={}",
                    //    boneInfo,
                    //    vertexIndex,
                    //    i,
                    //    weight));
                    continue;
                }
            }

            vb.addJoint(joint->m_jointIndex, weight);

            if (false && m_debug) {
                KI_INFO_OUT(fmt::format(
                    "ASSIMP: JOINT_VERTEX_BIND {}, vertex={}, vertexJoints={}, vertexWeights={}",
                    boneInfo,
                    vertexIndex,
                    vb.m_jointIds,
                    vb.m_weights));
            }
        }
    }
}
