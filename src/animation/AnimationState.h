#pragma once

#include <array>
#include <stdint.h>

#include "asset/SphereVolume.h"

#include "pool/NodeHandle.h"

namespace animation {
    struct AnimationState {
        struct Play {
            double m_startTime{ -1.f };
            float m_blendTime{ 0.f };
            float m_speed{ 1.f };

            int16_t m_clipIndex{ -1 };

            bool m_repeat : 1 { false };
            bool m_active : 1 { false };
        };

        pool::NodeHandle m_handle;
        uint16_t m_index;

        Play m_current;
        Play m_next;
        Play m_pending;

        // Animated bounding volume calculated from rig node positions
        SphereVolume m_animatedVolume{ 0.f };
        bool m_volumeDirty{ false };

        // Ground contact Y offset from foot sockets (local model space)
        // Represents lowest foot position for terrain placement
        float m_groundOffsetY{ 0.f };
        bool m_groundOffsetDirty{ false };
    };
}
