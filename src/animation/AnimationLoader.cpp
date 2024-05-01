#include "AnimationLoader.h"

#include <fmt/format.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "util/glm_format.h"
#include "util/Util.h"
#include "util/Log.h"

#include "RigContainer.h"
#include "Animation.h"
#include "BoneChannel.h"

#include "util/assimp_util.h"

namespace animation {
    AnimationLoader::AnimationLoader() = default;
    AnimationLoader::~AnimationLoader() = default;

    void AnimationLoader::loadAnimations(
        animation::RigContainer& rig,
        const std::string& name,
        const std::string& filePath)
    {
        KI_INFO_OUT(fmt::format("ASSIMP: FILE path={}", filePath));

        if (!util::fileExists(filePath)) {
            throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", filePath) };
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(
            filePath,
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
            filePath,
            scene->mNumMeshes,
            scene->mNumAnimations,
            scene->mNumSkeletons,
            scene->mNumMaterials,
            scene->mNumTextures))

        loadAnimations(rig, scene);
    }

    void AnimationLoader::loadAnimations(
        animation::RigContainer& rig,
        const aiScene* scene)
    {
        if (scene->mNumAnimations == 0) return;

        for (size_t index = 0; index < scene->mNumAnimations; index++) {
            auto animation = loadAnimation(
                rig,
                scene,
                scene->mAnimations[index]);
            rig.addAnimation(std::move(animation));
        }
    }

    std::unique_ptr<animation::Animation> AnimationLoader::loadAnimation(
        animation::RigContainer& rig,
        const aiScene* scene,
        const aiAnimation* anim)
    {
        KI_INFO_OUT(fmt::format(
            "ASSIMP: ANIM anim={}, duration={}, ticksPerSec={}, channels={}",
            anim->mName.C_Str(),
            anim->mDuration,
            anim->mTicksPerSecond,
            anim->mNumChannels));

        auto animation = std::make_unique<animation::Animation>(anim);

        animation->m_channels.reserve(anim->mNumChannels);
        for (size_t channelIdx = 0; channelIdx < anim->mNumChannels; ++channelIdx)
        {
            const aiNodeAnim* channel = anim->mChannels[channelIdx];
            KI_INFO_OUT(fmt::format(
                "ASSIMP: CHANNEL anim={}, channel={}, node={}, posKeys={}, rotKeys={}, scalingKeys={}",
                anim->mName.C_Str(),
                channelIdx,
                channel->mNodeName.C_Str(),
                channel->mNumPositionKeys,
                channel->mNumRotationKeys,
                channel->mNumScalingKeys));

            auto channelId = animation->addChannel(channel);
            auto& bc = animation->getChannel(channelId);
            bc.m_nodeId = rig.findNodeId(bc.m_nodeName);

            bc.m_positionKeys.reserve(channel->mNumPositionKeys);
            for (size_t i = 0; i < channel->mNumPositionKeys; i++) {
                bc.m_positionKeys.emplace_back(channel->mPositionKeys[i]);
            }

            bc.m_rotationKeys.reserve(channel->mNumRotationKeys);
            for (size_t i = 0; i < channel->mNumRotationKeys; i++) {
                bc.m_rotationKeys.emplace_back(channel->mRotationKeys[i]);
            }

            bc.m_scaleKeys.reserve(channel->mNumScalingKeys);
            for (size_t i = 0; i < channel->mNumScalingKeys; i++) {
                bc.m_scaleKeys.emplace_back(channel->mScalingKeys[i]);
            }
        }

        return animation;
    }
}
