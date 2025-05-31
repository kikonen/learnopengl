#pragma once

#include <string>
#include <array>
#include <chrono>

namespace ki {
    constexpr size_t FPS_FRAMES = 20;

    class FpsCounter {
    public:
        FpsCounter();

        void startFame();

        void endFame(float elapsedSecs);

        float getAvgFps() const noexcept
        {
            return m_fpsAvg;
        }

        float getAvgMillis() const noexcept
        {
            return m_millisAvg;
        }

        bool isUpdate() const noexcept
        {
            return m_avgIndex == 0;
        }

        std::string formatSummary(std::string_view title) const noexcept;

    private:
        void updateAvg();

    private:
        // NOTE KI moving avg of render time and fps
        size_t m_avgIndex{ 0 };
        std::array<float, FPS_FRAMES> m_fpsSecs{ 0.f };
        std::array<float, FPS_FRAMES> m_renderSecs{ 0.f };

        std::chrono::system_clock::time_point m_frameStart;
        std::chrono::system_clock::time_point m_frameEnd;

        float m_fpsAvg;
        float m_millisAvg;
    };
}
