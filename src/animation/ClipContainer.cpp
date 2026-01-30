#include "ClipContainer.h"

#include <algorithm>

#include <fmt/format.h>

#include "util/Log.h"

namespace {
    constexpr float DEF_FPS = 25.f;

    // Convert tick value to index in unified keyTimes
    // For start: returns first keyTime >= tickValue
    // For end: returns last keyTime <= tickValue
    uint16_t tickToStartIndex(
        const std::vector<float>& keyTimes,
        float tickValue)
    {
        if (keyTimes.empty()) return 0;

        auto it = std::lower_bound(keyTimes.begin(), keyTimes.end(), tickValue);
        if (it == keyTimes.end()) {
            return static_cast<uint16_t>(keyTimes.size() - 1);
        }
        return static_cast<uint16_t>(std::distance(keyTimes.begin(), it));
    }

    // Returns exclusive end index (first frame NOT in clip)
    // This matches the convention in RigNodeChannel where lastFrame is exclusive
    uint16_t tickToEndIndex(
        const std::vector<float>& keyTimes,
        float tickValue)
    {
        if (keyTimes.empty()) return 0;

        // upper_bound finds first element > tickValue
        // This gives us the exclusive end (first frame after the clip)
        auto it = std::upper_bound(keyTimes.begin(), keyTimes.end(), tickValue);
        return static_cast<uint16_t>(std::distance(keyTimes.begin(), it));
    }
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
            "ASSIMP::ADD_ANIMATION: name={}, index={}",
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

            // Convert tick-based frame numbers to unified timeline indices
            // Clip firstFrame/lastFrame from metadata are tick values, not array indices
            if (!anim->m_channels.empty() && !clip.m_single) {
                const auto& keyTimes = anim->m_channels[0].getKeyTimes();

                // Store original tick values for debug
                uint16_t tickFirst = clip.m_firstFrame;
                uint16_t tickLast = clip.m_lastFrame;

                // Convert ticks to indices
                clip.m_firstFrame = tickToStartIndex(keyTimes, static_cast<float>(tickFirst));
                clip.m_lastFrame = tickToEndIndex(keyTimes, static_cast<float>(tickLast));

                // Ensure lastFrame doesn't exceed max
                // NOTE KI getMaxFrame() returns inclusive max (size-1), but lastFrame is exclusive
                // so valid lastFrame range is [firstFrame+1, size]
                uint16_t maxExclusive = static_cast<uint16_t>(keyTimes.size());
                if (clip.m_lastFrame > maxExclusive) {
                    clip.m_lastFrame = maxExclusive;
                }
                if (clip.m_firstFrame >= clip.m_lastFrame) {
                    clip.m_firstFrame = clip.m_lastFrame > 0 ? clip.m_lastFrame - 1 : 0;
                }

                // Set tick values from unified timeline for LUT generation
                // These must match how clipDuration is calculated (from unified timeline)
                // so that runtime normalizedTime maps correctly to LUT entries
                clip.m_firstTick = keyTimes[clip.m_firstFrame];
                clip.m_lastTick = clip.m_lastFrame > 0 ? keyTimes[clip.m_lastFrame - 1] : keyTimes[clip.m_firstFrame];

                // Debug: show actual keyTime values at boundaries
                float nextKeyTime = clip.m_lastFrame < keyTimes.size() ? keyTimes[clip.m_lastFrame] : -1.f;

                KI_DEBUG(fmt::format(
                    "ASSIMP::CLIP_TICK_TO_INDEX: name={}, ticks=[{},{}] -> indices=[{},{}) keyTimes=[{:.1f},{:.1f}] next={:.1f}, size={}",
                    clip.m_uniqueName,
                    tickFirst, tickLast,
                    clip.m_firstFrame, clip.m_lastFrame,
                    clip.m_firstTick, clip.m_lastTick, nextKeyTime,
                    keyTimes.size()));
            }
            else if (clip.m_single && !anim->m_channels.empty()) {
                // Single clip uses entire animation - set tick range from original tracks
                const auto& channel = anim->m_channels[0];
                const auto& origTimes = channel.getOrigPositionTimes();
                if (!origTimes.empty()) {
                    clip.m_firstTick = origTimes.front();
                    clip.m_lastTick = origTimes.back();
                } else {
                    // Fallback to animation duration
                    clip.m_firstTick = 0.f;
                    clip.m_lastTick = anim->m_duration;
                }
            }
            else if (auto max = anim->getMaxFrame(); clip.m_lastFrame > max) {
                KI_WARN_OUT(fmt::format(
                    "ASSIMP::CLIP_OUT_OF_BOUNDS: name={}, index={}, animName={}, animIndex={}, range=[{},{}], max={}",
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

        KI_DEBUG(fmt::format(
            "ASSIMP::ADD_CLIP: name={}, index={}, animName={}, animIndex={}, range=[{},{}]",
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
            lut.generate(channel, clip.m_uniqueName, clip.m_firstTick, clip.m_lastTick);
        }

        KI_DEBUG(fmt::format(
            "ASSIMP::CLIP_LUT: clip={}, channels={}, nodes={}",
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

