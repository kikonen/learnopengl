#include "AnimationImporter.h"

#include <fmt/format.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "util/glm_format.h"
#include "util/util.h"
#include "util/file.h"
#include "util/Log.h"
#include "util/assimp_util.h"

#include "animation/Rig.h"
#include "animation/RigNode.h"
#include "animation/Animation.h"
#include "animation/RigNodeChannel.h"
#include "animation/RigNodeChannelBuilder.h"
#include "animation/Clip.h"
#include "animation/Metadata.h"

#include "MetadataImporter.h"

namespace mesh_set
{
    AnimationImporter::AnimationImporter() = default;
    AnimationImporter::~AnimationImporter() = default;

    void AnimationImporter::loadAnimations(
        animation::Rig& rig,
        const std::string& uniquePrefix,
        const std::string& filePath)
    {
        KI_INFO_OUT(fmt::format("ASSIMP::LOAD_FILE path={}", filePath));

        // NOTE KI animations cannot exist in empty rig
        if (rig.empty()) return;

        if (!util::fileExists(filePath)) {
            throw AnimationNotFoundError{ fmt::format("ASSIMP::FILE_NOT_FOUND: {}", filePath) };
        }

        Assimp::Importer importer;

        // NOTE KI likely should match AssimpImporter on these
        const aiScene* scene = importer.ReadFile(
            filePath,
            //aiProcess_GenNormals |
            //aiProcess_GenSmoothNormals |
            //aiProcess_ForceGenNormals |
            //aiProcess_FixInfacingNormals |
            //aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            //aiProcess_ImproveCacheLocality |
            aiProcess_LimitBoneWeights |
            aiProcess_RemoveRedundantMaterials |
            aiProcess_GenUVCoords |
            //aiProcess_SortByPType |
            0);

        // If the import failed, report it
        if (!scene) {
            KI_ERROR(importer.GetErrorString());
            return;
        }

        KI_INFO_OUT(fmt::format(
            "ASSIMP::SCENE scene={}, meshes={}, anims={}, materials={}, textures={}",
            filePath,
            scene->mNumMeshes,
            scene->mNumAnimations,
            scene->mNumMaterials,
            scene->mNumTextures))

        loadAnimations(rig, uniquePrefix, filePath, scene);
    }

    void AnimationImporter::loadAnimations(
        animation::Rig& rig,
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

        MetadataImporter metadataImporter{};
        const auto metadata = metadataImporter.load(filePath);
        if (metadata) {
            for (auto& clip : metadata->m_clips) {
                clip.m_uniqueName = fmt::format(
                    "{}:{}",
                    uniquePrefix.empty() ? "<ANIM_NAME_MISSING>" : uniquePrefix,
                    clip.m_uniqueName);

                // TODO KI clip sequences seem to be stored like
                // 0 - 48, 48 - 98, 98 - ...
                if (clip.m_firstFrame > 0 && clip.m_firstFrame < clip.m_lastFrame) {
                    clip.m_firstFrame++;
                }
                clip.m_animationName = uniquePrefix + ":" + clip.m_animationName;
                clipContainer.addClip(clip);
            }
        }

        // NOTE KI register anims without unique name with their given name
        for (auto animIndex : animationIndeces) {
            const auto& animation = *clipContainer.m_animations[animIndex];

            if (animation.getClipCount() > 1) {
                // NOTE KI assume all channels are consistent
                const animation::RigNodeChannel* prev{ nullptr };
                for (const auto& channel : animation.m_channels) {
                    if (prev) {
                        // NOTE KI *NOT* true always
                        //assert(channel.m_positionKeyTimes.size() == prev->m_positionKeyTimes.size());
                        //assert(channel.m_rotationKeyTimes.size() == prev->m_rotationKeyTimes.size());
                        //assert(channel.m_scaleKeyTimes.size() == prev->m_scaleKeyTimes.size());
                    }

                    // NOTE KI *NOT* true always
                    //assert(channel.m_positionKeyTimes.size() == channel.m_rotationKeyTimes.size());
                    //assert(channel.m_positionKeyTimes.size() == channel.m_scaleKeyTimes.size());

                    prev = &channel;
                }
            }
            else {
                animation::Clip clip;
                clip.m_uniqueName = animation.m_uniqueName;
                clip.m_animationName = animation.m_uniqueName;
                clip.m_lastFrame = animation.getMaxFrame();
                clipContainer.addClip(clip);
            }
        }
    }

    std::unique_ptr<animation::Animation> AnimationImporter::loadAnimation(
        animation::Rig& rig,
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
            "ASSIMP::ANIM anim={}, name={}, duration={}, ticksPerSec={}, channels={}",
            animation->m_index,
            animation->m_name,
            animation->m_duration,
            animation->m_ticksPerSecond,
            anim->mNumChannels));

        animation->m_channels.reserve(anim->mNumChannels);
        for (size_t channelIdx = 0; channelIdx < anim->mNumChannels; ++channelIdx)
        {
            const aiNodeAnim* channel = anim->mChannels[channelIdx];
            KI_INFO(fmt::format(
                "ASSIMP::CHANNEL anim={}, channel={}, node={}, posKeys={}, rotKeys={}, scalingKeys={}",
                animation->m_index,
                channelIdx,
                assimp_util::normalizeName(channel->mNodeName),
                channel->mNumPositionKeys,
                channel->mNumRotationKeys,
                channel->mNumScalingKeys));

            auto& rigNodeChannel = animation->addChannel({ channel });
            auto* rigNode = rig.findNode(rigNodeChannel.m_nodeName);

            if (!rigNode) {
                //if (rigNodeChannel.m_nodeName.starts_with("root_")) {
                //    rigNode = rig.findNode("root");
                //}
            }

            if (rigNode) {
                KI_INFO(fmt::format(
                    "ASSIMP::CHANNEL_BIND_NODE - channel={}, node={}",
                    rigNodeChannel.m_nodeName,
                    rigNode->m_name
                ));
                animation->bindNode(rigNodeChannel.m_index, rigNode->m_index);
            }
            else {
                KI_WARN_OUT(fmt::format(
                    "ASSIMP::CHANNEL_MISSING_NODE - channel={}",
                    rigNodeChannel.m_nodeName
                ));
            }

            animation::RigNodeChannelBuilder builder{ rigNodeChannel };

            builder.reservePositionKeys(channel->mNumPositionKeys);
            for (size_t i = 0; i < channel->mNumPositionKeys; i++) {
                builder.addPositionKey(channel->mPositionKeys[i]);
            }

            builder.reserveRotationKeys(channel->mNumRotationKeys);
            for (size_t i = 0; i < channel->mNumRotationKeys; i++) {
                builder.addRotationKey(channel->mRotationKeys[i]);
            }

            builder.reserveScaleKeys(channel->mNumScalingKeys);
            for (size_t i = 0; i < channel->mNumScalingKeys; i++) {
                builder.addScaleKey(channel->mScalingKeys[i]);
            }

            // Unify position/rotation/scale to common timeline for optimized interpolation
            builder.unifyKeyTimes();
        }

        return animation;
    }
}
