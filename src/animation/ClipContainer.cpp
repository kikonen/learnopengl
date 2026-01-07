#include "ClipContainer.h"

#include <fmt/format.h>

#include "util/Log.h"

namespace {
    constexpr float DEF_FPS = 25.f;
}

namespace animation {
    const animation::Animation* ClipContainer::getAnimation(uint16_t animationIndex) const
    {
        if (animationIndex < 0 || animationIndex >= m_animations.size()) return nullptr;
        return m_animations[animationIndex].get();
    }

    const animation::Clip* ClipContainer::getClip(uint16_t clipIndex) const
    {
        if (clipIndex < 0 || clipIndex >= m_clips.size()) return nullptr;
        return &m_clips[clipIndex];
    }

    animation::Animation* ClipContainer::modifyAnimation(uint16_t animationIndex)
    {
        if (animationIndex < 0 || animationIndex >= m_animations.size()) return nullptr;
        return m_animations[animationIndex].get();
    }

    animation::Clip* ClipContainer::modifyClip(uint16_t clipIndex)
    {
        if (clipIndex < 0 || clipIndex >= m_clips.size()) return nullptr;
        return &m_clips[clipIndex];
    }

    uint16_t ClipContainer::addAnimation(
        std::unique_ptr<animation::Animation> src)
    {
        uint16_t index = static_cast<uint16_t>(m_animations.size());
        m_animations.push_back(std::move(src));

        auto* anim = m_animations[index].get();
        anim->m_index = index;

        KI_INFO_OUT(fmt::format(
            "ASSIMP: ADD_ANIMATION: name={}, index={}",
            anim->m_name,
            anim->m_index));

        return index;
    }

    uint16_t ClipContainer::addClip(const animation::Clip& src)
    {
        uint16_t index = static_cast<uint16_t>(m_clips.size());
        m_clips.push_back(src);

        auto& clip = m_clips[index];
        clip.m_index = index;

        assert(clip.m_firstFrame <= clip.m_lastFrame);

        const auto* anim = findAnimation(src.m_animationName);
        if (anim) {
            m_animations[anim->m_index]->m_clipCount++;

            clip.m_animationIndex = anim->m_index;

            if (auto max = anim->getMaxFrame(); clip.m_lastFrame > max) {
                KI_WARN_OUT(fmt::format(
                    "ASSIMP: CLIP_OUT_OF_BOUNDS: name={}, index={}, animName={}, animIndex={}, range=[{},{}], max={}",
                    clip.m_uniqueName,
                    clip.m_index,
                    clip.m_animationName,
                    clip.m_animationIndex,
                    clip.m_firstFrame,
                    clip.m_lastFrame,
                    max));
                clip.m_lastFrame = max;
                if (clip.m_firstFrame > clip.m_lastFrame) {
                    clip.m_firstFrame = clip.m_lastFrame;
                }
            }

            clip.m_duration = clip.m_single ? anim->m_duration : anim->getClipDuration(clip.m_firstFrame, clip.m_lastFrame);
            clip.m_durationSecs = animationTimeToSecs(clip.m_animationIndex, clip.m_duration);
        }

        KI_INFO_OUT(fmt::format(
            "ASSIMP: ADD_CLIP: name={}, index={}, animName={}, animIndex={}, range=[{},{}]",
            clip.m_uniqueName,
            clip.m_index,
            clip.m_animationName,
            clip.m_animationIndex,
            clip.m_firstFrame,
            clip.m_lastFrame));

        // Generate per-clip LUTs for O(1) sampling
        generateClipLUTs(index);

        return index;
    }

    void ClipContainer::generateClipLUTs(uint16_t clipIndex)
    {
        const auto& clip = m_clips[clipIndex];
        if (clip.m_animationIndex < 0) return;

        const auto& animation = *m_animations[clip.m_animationIndex];

        // Ensure we have space for this clip's LUTs
        if (m_clipLUTs.size() <= clipIndex) {
            m_clipLUTs.resize(clipIndex + 1);
        }

        // Get max node index from animation's nodeToChannel mapping
        const auto& channels = animation.m_channels;
        if (channels.empty()) return;

        // Find max node index
        uint16_t maxNodeIndex = 0;
        for (const auto& channel : channels) {
            const auto nodeIndex = channel.getNodeIndex();
            if (nodeIndex >= 0) {
                maxNodeIndex = std::max(maxNodeIndex, static_cast<uint16_t>(nodeIndex));
            }
        }

        // Resize to accommodate all node indices
        m_clipLUTs[clipIndex].resize(maxNodeIndex + 1);

        // Generate LUT for each channel
        for (const auto& channel : channels) {
            const auto nodeIndex = channel.getNodeIndex();
            if (nodeIndex < 0) continue;

            auto& lut = m_clipLUTs[clipIndex][nodeIndex];
            lut.generate(channel, clip.m_firstFrame, clip.m_lastFrame);
        }

        KI_INFO_OUT(fmt::format(
            "ASSIMP: CLIP_LUT: clip={}, channels={}, nodes={}",
            clip.m_uniqueName,
            channels.size(),
            maxNodeIndex + 1));
    }

    const ClipChannelLUT* ClipContainer::getChannelLUT(
        uint16_t clipIndex,
        uint16_t nodeIndex) const noexcept
    {
        if (clipIndex >= m_clipLUTs.size()) return nullptr;

        const auto& clipLUTs = m_clipLUTs[clipIndex];
        if (nodeIndex >= clipLUTs.size()) return nullptr;

        const auto& lut = clipLUTs[nodeIndex];
        return lut.empty() ? nullptr : &lut;
    }

    const animation::Animation* ClipContainer::findAnimation(const std::string& name) const
    {
        const auto& it = std::find_if(
            m_animations.begin(),
            m_animations.end(),
            [&name](const auto& anim) { return anim->m_uniqueName == name;  });
        if (it == m_animations.end()) return nullptr;
        return it->get();
    }

    const animation::Clip* ClipContainer::findClip(ki::sid_t id) const
    {
        const auto& it = std::find_if(
            m_clips.begin(),
            m_clips.end(),
            [&id](const auto& clip) { return clip.m_id == id;  });
        if (it == m_clips.end()) return nullptr;
        return &(*it);
    }

    animation::Clip* ClipContainer::findClipByUniqueName(
        const std::string& uniqueName)
    {
        const auto& it = std::find_if(
            m_clips.begin(),
            m_clips.end(),
            [&uniqueName](const auto& clip) { return clip.m_uniqueName == uniqueName;  });
        if (it == m_clips.end()) return nullptr;
        return &(*it);
    }

    uint16_t ClipContainer::getClipCount(uint16_t animationIndex) const
    {
        return m_animations[animationIndex]->m_clipCount;
    }

    float ClipContainer::getAnimationTimeTicks(
        uint16_t clipIndex,
        double animationStartTime,
        double currentTime,
        float speed) const
    {
        const auto& clip = m_clips[clipIndex];
        const auto& animation = *m_animations[clip.m_animationIndex];

        float animationTimeTicks;
        {
            const float animationTimeSecs = (float)(currentTime - animationStartTime);
            const float ticksPerSecond = animation.m_ticksPerSecond != 0.f ? animation.m_ticksPerSecond : DEF_FPS;
            const float timeInTicks = animationTimeSecs * ticksPerSecond * speed;
            animationTimeTicks = fmod(timeInTicks, clip.m_duration);

            //std::cout << fmt::format(
            //    "time={}, secs={}, ticksSec={}, ticks={}, duration={}\n",
            //    currentTime, animationTimeSecs, ticksPerSecond, timeInTicks, animation->m_duration);
        }
        return animationTimeTicks;
    }

    float ClipContainer::animationTimeToSecs(
        uint16_t animationIndex,
        float animationTime) const
    {
        const auto& animation = *m_animations[animationIndex];
        const float ticksPerSecond = animation.m_ticksPerSecond != 0.f ? animation.m_ticksPerSecond : DEF_FPS;
        return animationTime / ticksPerSecond;
    }
}

