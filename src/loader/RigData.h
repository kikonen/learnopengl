#pragma once

#include <string>

#include <glm/glm.hpp>

#include "SocketData.h"
#include "AnimationData.h"

namespace loader
{
    inline const std::string RIG_ALIAS_ANY = "*";

    struct RigData
    {
        std::string name;
        std::vector<SocketData> sockets;
        std::vector<AnimationData> animations;

        inline bool isAny() const noexcept
        {
            return name == RIG_ALIAS_ANY;
        }

        inline bool match(const std::string dstName) const noexcept
        {
            return name == dstName || isAny();
        }
    };
}
