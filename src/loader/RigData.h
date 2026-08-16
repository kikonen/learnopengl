#pragma once

#include <string>

#include <glm/glm.hpp>

#include "animation/AnimationPath.h"

#include "SocketData.h"
#include "AnimationData.h"

namespace loader
{
    inline const std::string RIG_ALIAS_ANY = "*";

    struct RigData
    {
        std::string name;
        std::string alias;

        std::vector<SocketData> sockets;
        std::vector<AnimationData> animations;

        inline bool isAny() const noexcept
        {
            return alias == RIG_ALIAS_ANY;
        }

        inline bool match(const std::string dstName) const noexcept
        {
            return name == dstName || isAny();
        }

        std::vector<animation::AnimationPath> getAnimationPaths() const noexcept
        {
            std::vector<animation::AnimationPath> paths;
            for (const auto& animation : animations) {
                if (animation.name.empty()) continue;
                if (animation.path.empty()) continue;
                paths.push_back({ animation.name, animation.path });
            }
            return paths;
        }
    };
}
