#include "ClipContainer.h"

#include <fmt/format.h>

#include "util/Log.h"

namespace animation {
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

        const auto* anim = findAnimation(clip.m_animationName);
        if (anim) {
            m_animations[anim->m_index]->m_clipCount++;

            clip.m_animationIndex = anim->m_index;

            if (auto max = anim->getMaxFrame(); clip.m_lastFrame > max) {
                KI_WARN_OUT(fmt::format(
                    "ASSIMP: CLIP_OUT_OF_BOUNDS: name={}, index={}, animName={}, animIndex={}, range=[{},{}], max={}",
                    clip.m_name,
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

            clip.m_duration = anim->getClipDuration(clip.m_firstFrame, clip.m_lastFrame);
        }

        KI_INFO_OUT(fmt::format(
            "ASSIMP: ADD_CLIP: name={}, index={}, animName={}, animIndex={}, range=[{},{}]",
            clip.m_name,
            clip.m_index,
            clip.m_animationName,
            clip.m_animationIndex,
            clip.m_firstFrame,
            clip.m_lastFrame));

        return index;
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

    const animation::Clip* ClipContainer::findClip(const std::string& name) const
    {
        const auto& it = std::find_if(
            m_clips.begin(),
            m_clips.end(),
            [&name](const auto& clip) { return clip.m_name == name;  });
        if (it == m_clips.end()) return nullptr;
        return &(*it);
    }

    uint16_t ClipContainer::getClipCount(uint16_t animationIndex) const
    {
        return m_animations[animationIndex]->m_clipCount;
    }
}

