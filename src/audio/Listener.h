#pragma once

#include <glm/glm.hpp>

#include <AL/al.h>

#include "ki/size.h"

#include "size.h"

struct NodeState;

namespace audio
{
    struct Listener {
        Listener() {}
        Listener(Listener& o) = delete;
        Listener(const Listener& o) = delete;
        Listener(Listener&& o) noexcept;
        ~Listener();

        Listener& operator=(Listener& o) = delete;
        Listener& operator=(Listener&& o) noexcept;

        void updateActive(const NodeState& state) const;

        float m_gain{ 1.f };
    };
}
