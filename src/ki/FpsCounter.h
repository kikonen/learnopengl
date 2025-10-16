#pragma once

#include <string>
#include <array>
#include <chrono>

namespace ki {
    constexpr size_t AVG_FRAMES = 20;

    class FpsCounter {
    public:
        FpsCounter();

        void startRender();
        void endRender();

        void startFrame();
        void endFrame();

        float getAvgFps() const noexcept
        {
            return m_fpsAvg;
        }

        float getRenderAvg() const noexcept
        {
            return m_renderAvg;
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
        std::array<float, AVG_FRAMES> m_fpsSecs{ 0.f };
        std::array<float, AVG_FRAMES> m_renderSecs{ 0.f };

        std::chrono::system_clock::time_point m_renderStart;
        std::chrono::system_clock::time_point m_renderEnd;

        std::chrono::system_clock::time_point m_frameStart;
        std::chrono::system_clock::time_point m_frameEnd;

        float m_fpsAvg;
        float m_renderAvg;
    };
}
