#include "AnimationLoader.h"

#include <fmt/format.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "util/glm_format.h"
#include "util/Util.h"
#include "util/Log.h"
#include "util/assimp_util.h"

#include "RigContainer.h"
#include "Animation.h"
#include "BoneChannel.h"

#include "MetadataLoader.h"
#include "Metadata.h"
#include "Clip.h"

namespace animation {
    AnimationLoader::AnimationLoader() = default;
    AnimationLoader::~AnimationLoader() = default;

    void AnimationLoader::loadAnimations(
        animation::RigContainer& rig,
        const std::string& uniquePrefix,
        const std::string& filePath)
    {
        KI_INFO_OUT(fmt::format("ASSIMP: FILE path={}", filePath));

        // NOTE KI animations cannot exist in empty rig
        if (rig.empty()) return;

        if (!util::fileExists(filePath)) {
            throw AnimationNotFoundError{ fmt::format("FILE_NOT_EXIST: {}", filePath) };
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
            scene->mNumMaterials,
            scene->mNumTextures))

        loadAnimations(rig, uniquePrefix, filePath, scene);
    }

    void AnimationLoader::loadAnimations(
        animation::RigContainer& rig,
        const std::string& uniquePrefix,
        const std::string& filePath,
        const aiScene* scene)
    {
        if (scene->mNumAnimations == 0) return;

        auto& clipContainer = rig.m_clipContainer;

        std::vector<uint16_t> animationIndeces;

        for (size_t index = 0; index < scene->mNumAnimations; index++) {
            auto animation = loadAnimation(
                rig,
                static_cast<int16_t>(clipContainer.m_animations.size()),
                uniquePrefix,
                scene,
                scene->mAnimations[index]);
            auto animIndex = clipContainer.addAnimation(std::move(animation));
            animationIndeces.push_back(animIndex);
        }

        animation::MetadataLoader metadataLoader{};
        const auto metadata = metadataLoader.load(filePath);
        if (metadata) {
            for (auto& clip : metadata->m_clips) {
                clip.m_animationName = uniquePrefix + "_" + clip.m_animationName;
                clipContainer.addClip(clip);
            }
        }

        // NOTE KI register anims without unique name with their given name
        for (auto animIndex : animationIndeces) {
            if (!clipContainer.hasClips(animIndex)) {
                const auto& animation = *clipContainer.m_animations[animIndex];
                animation::Clip clip;
                clip.m_name = animation.m_name;
                clip.m_animationName = animation.m_uniqueName;
                clip.m_lastFrame = animation.m_duration;
                clipContainer.addClip(clip);
            }
        }
    }

    std::unique_ptr<animation::Animation> AnimationLoader::loadAnimation(
        animation::RigContainer& rig,
        int16_t animIndex,
        const std::string& uniquePrefix,
        const aiScene* scene,
        const aiAnimation* anim)
    {
        auto animation = std::make_unique<animation::Animation>(
            anim,
            uniquePrefix);
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
                "ASSIMP: CHANNEL anim={}, channel={}, joint={}, posKeys={}, rotKeys={}, scalingKeys={}",
                animation->m_index,
                channelIdx,
                channel->mNodeName.C_Str(),
                channel->mNumPositionKeys,
                channel->mNumRotationKeys,
                channel->mNumScalingKeys));

            auto& bc = animation->addChannel({ channel });
            auto* rigJoint = rig.findJoint(bc.m_jointName);
            if (rigJoint) {
                KI_INFO_OUT(fmt::format(
                    "ASSIMP: CHANNEL_BIND_JOINT - channel={}, joint={}",
                    bc.m_jointName,
                    rigJoint->m_name
                ));
                animation->bindJoint(bc.m_index, rigJoint->m_index);
            }
            else {
                KI_WARN_OUT(fmt::format(
                    "ASSIMP: CHANNEL_MISSING_JOINT - channeö={}",
                    bc.m_jointName
                ));
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
