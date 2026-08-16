#pragma once

#include <string>

namespace animation
{
    struct AnimationPath
    {
        std::string animationPrefix;
        std::string path;

        bool empty() const noexcept
        {
            return animationPrefix.empty() || path.empty();
        }
    };
}
