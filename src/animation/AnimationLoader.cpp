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
        const std::string& namePrefix,
        const std::string& filePath)
    {
        KI_INFO_OUT(fmt::format("ASSIMP: FILE path={}", filePath));

        // NOTE KI animations cannot exist in empty rig
        if (rig.empty()) return;

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

        loadAnimations(rig, namePrefix, scene);
    }

    void AnimationLoader::loadAnimations(
        animation::RigContainer& rig,
        const std::string& namePrefix,
        const aiScene* scene)
    {
        if (scene->mNumAnimations == 0) return;


        for (size_t index = 0; index < scene->mNumAnimations; index++) {
            auto animation = loadAnimation(
                rig,
                static_cast<int16_t>(rig.m_animations.size()),
                namePrefix,
                scene,
                scene->mAnimations[index]);
            rig.addAnimation(std::move(animation));
        }
    }

    std::unique_ptr<animation::Animation> AnimationLoader::loadAnimation(
        animation::RigContainer& rig,
        int16_t animIndex,
        const std::string& namePrefix,
        const aiScene* scene,
        const aiAnimation* anim)
    {
        auto animation = std::make_unique<animation::Animation>(
            anim,
            namePrefix);
        animation->m_index = animIndex;

        KI_INFO_OUT(fmt::format(
            "ASSIMP: ANIM anim={}, name={}, duration={}, ticksPerSec={}, channels={}",
            animation->m_index,
            animation->m_name,
            animation->m_duration,
            animation->m_ticksPerSecond,
            anim->mNumChannels));

        animation->m_channels.reserve(anim->mNumChannels);
        for (size_t channelIdx = 0; channelIdx < anim->mNumChannels; ++channelIdx)
        {
            const aiNodeAnim* channel = anim->mChannels[channelIdx];
            KI_INFO_OUT(fmt::format(
                "ASSIMP: CHANNEL anim={}, channel={}, node={}, posKeys={}, rotKeys={}, scalingKeys={}",
                animation->m_index,
                channelIdx,
                channel->mNodeName.C_Str(),
                channel->mNumPositionKeys,
                channel->mNumRotationKeys,
                channel->mNumScalingKeys));

            auto& bc = animation->addChannel({ channel });
            auto* rigNode = rig.findNode(bc.m_nodeName);
            if (rigNode) {
                animation->bindNode(bc.m_index, rigNode->m_index);
            }

            bc.reservePositionKeys(channel->mNumPositionKeys);
            for (size_t i = 0; i < channel->mNumPositionKeys; i++) {
                bc.addPositionKey(channel->mPositionKeys[i]);
            }

            bc.reserveRotationKeys(channel->mNumPositionKeys);
            for (size_t i = 0; i < channel->mNumRotationKeys; i++) {
                bc.addeRotationKey(channel->mRotationKeys[i]);
            }

            bc.reserveScaleKeys(channel->mNumPositionKeys);
            for (size_t i = 0; i < channel->mNumScalingKeys; i++) {
                bc.addeScaleKey(channel->mScalingKeys[i]);
            }
        }

        return animation;
    }
}
