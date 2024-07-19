#pragma once

#include <array>
#include <stdint.h>

#include "pool/NodeHandle.h"

namespace animation {
    struct AnimationState {
        struct Play {
            double m_startTime{ -1.f };
            float m_duration{ 0.f };
            float m_blendTime{ 0.f };
            float m_speed{ 1.f };

            int16_t m_clipIndex{ -1 };

            bool m_repeat : 1 { false };
            bool m_active : 1 { false };

            void updateActive(double currentTime) {
                m_active = m_startTime <= currentTime && m_startTime + m_duration > currentTime;
            }
        };

        pool::NodeHandle m_handle;
        uint16_t m_index;

        Play m_current;
        Play m_next;
        Play m_pending;
    };
}
