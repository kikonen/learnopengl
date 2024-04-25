#include "AnimationLoader.h"

#include <fmt/format.h>

#include <assimp/scene.h>

#include "util/glm_format.h"
#include "util/Util.h"
#include "util/Log.h"

#include "Animation.h"
#include "BoneChannel.h"

#include "util/assimp_util.h"

namespace animation {
    AnimationLoader::~AnimationLoader() = default;

    std::vector<std::unique_ptr<animation::Animation>> AnimationLoader::loadAnimations(
        animation::RigContainer& rig,
        aiScene* scene)
    {
        if (scene->mNumAnimations == 0) return {};

        std::vector<std::unique_ptr<animation::Animation>> animations;

        for (size_t index = 0; index < scene->mNumAnimations; index++) {
            auto animation = loadAnimation(
                rig,
                scene,
                scene->mAnimations[index]);
            animations.push_back(std::move(animation));
        }

        return animations;
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

            BoneChannel bc{ channel };

            for (size_t i = 0; i < channel->mNumPositionKeys; i++) {
                bc.m_positionKeys.emplace_back(channel->mPositionKeys[i]);
            }

            for (size_t i = 0; i < channel->mNumRotationKeys; i++) {
                bc.m_rotationKeys.emplace_back(channel->mRotationKeys[i]);
            }

            for (size_t i = 0; i < channel->mNumScalingKeys; i++) {
                bc.m_scalingKeys.emplace_back(channel->mScalingKeys[i]);
            }

            animation->m_channels.push_back(bc);
        }

        return animation;
    }
}
