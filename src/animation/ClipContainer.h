#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

#include "Animation.h"
#include "Clip.h"

namespace animation {
    struct Animation;

    struct ClipContainer
    {
        std::vector<std::unique_ptr<animation::Animation>> m_animations;
        std::vector<animation::Clip> m_clips;

        const animation::Animation* getAnimation(uint16_t animationIndex) const;
        const animation::Clip* getClip(uint16_t clipIndex) const;

        animation::Animation* modifyAnimation(uint16_t animationIndex);
        animation::Clip* modifyClip(uint16_t clipIndex);

        uint16_t addAnimation(
            std::unique_ptr<animation::Animation> take);

        uint16_t addClip(const animation::Clip& clip);

        const animation::Animation* findAnimation(const std::string& name) const;
        const animation::Clip* findClip(ki::sid_t id) const;
        animation::Clip* findClipByUniqueName(const std::string& uniqueName);

        uint16_t getClipCount(uint16_t animationIndex) const;

        float getAnimationTimeTicks(
            uint16_t clipIndex,
            double animationStartTime,
            double currentTime,
            float speed) const;

    private:
        float animationTimeToSecs(
            uint16_t animationIndex,
            float animationTime) const;
    };
}
