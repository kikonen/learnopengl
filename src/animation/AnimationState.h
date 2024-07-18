#pragma once

#include <array>
#include <stdint.h>

#include "pool/NodeHandle.h"

namespace animation {
    struct AnimationState {
        struct Play {
            double m_startTime{ -1.f };
            double m_lastTime{ -1.f };

            int16_t m_clipIndex{ -1 };

            float m_speed{ 1.f };

            bool m_repeat : 1 { false };
            bool m_active : 1 { false };
        };

        pool::NodeHandle m_handle;
        uint16_t m_index;

        float m_blendDuration{ 0.f };
        Play m_current;
        Play m_next;
    };
}
