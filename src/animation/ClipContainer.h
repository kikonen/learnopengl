#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

#include "Animation.h"
#include "Clip.h"

namespace animation {
    struct Animation;

    struct ClipContainer {
        std::vector<std::unique_ptr<animation::Animation>> m_animations;
        std::vector<animation::Clip> m_clips;

        uint16_t addAnimation(
            std::unique_ptr<animation::Animation> take,
            bool registerClip);

        uint16_t addClip(const animation::Clip& clip);

        const animation::Animation* findAnimation(const std::string& name) const;
        const animation::Clip* findClip(const std::string& name) const;
    };
}
