#pragma once

#include <memory>
#include <vector>
#include <string>
#include <atomic>

#include <glm/glm.hpp>

#include "ki/size.h"
#include "ki/sid.h"

namespace debug {
    struct AnimationDebug {
        bool m_debugEnabled{ false };
        bool m_paused{ false };
        bool m_forceFirstFrame{ false };

        bool m_manualTime{ 0 };
        float m_currentTime{ 0 };

        int m_clipIndexA{ -1 };
        float m_startTimeA{ 0 };
        float m_speedA{ 1.f };

        bool m_blend{ false };
        float m_blendFactor{ 0.f };

        int m_clipIndexB{ -1 };
        float m_startTimeB{ 0 };
        float m_speedB{ 1.f };

        int m_jointIndex{ 0 };
        bool m_debugJointWeight{ false };

        bool m_showSockets{ false };

        void prepare();
    };
}
