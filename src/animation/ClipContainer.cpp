#include "ClipContainer.h"

namespace animation {
    uint16_t ClipContainer::addAnimation(
        std::unique_ptr<animation::Animation> src,
        bool registerClip)
    {
        uint16_t index = static_cast<uint16_t>(m_animations.size());
        m_animations.push_back(std::move(src));

        auto* anim = m_animations[index].get();
        anim->m_index = index;
        if (registerClip) {
            Clip clip;
            clip.m_animationName = anim->m_name;
            clip.m_lastFrame = anim->m_duration;

            addClip(clip);
        }

        return index;
    }

    uint16_t ClipContainer::addClip(const animation::Clip& src)
    {
        uint16_t index = static_cast<uint16_t>(m_clips.size());
        m_clips.push_back(src);

        auto& clip = m_clips[index];
        clip.m_index = index;

        const auto* anim = findAnimation(clip.m_animationName);
        if (anim) {
            clip.m_animationIndex = anim->m_index;
        }

        return index;
    }

    const animation::Animation* ClipContainer::findAnimation(const std::string& name) const
    {
        const auto& it = std::find_if(
            m_animations.begin(),
            m_animations.end(),
            [&name](const auto& anim) { return anim->m_name == name;  });
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
}

