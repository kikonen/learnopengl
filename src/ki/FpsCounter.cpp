#include "FpsCounter.h"

namespace ki {
    FpsCounter::FpsCounter()
    {
    }

    void FpsCounter::startFame()
    {
        m_frameStart = std::chrono::system_clock::now();

    }

    void FpsCounter::endFame(float elapsedSecs)
    {
        m_frameEnd = std::chrono::system_clock::now();
        std::chrono::duration<float> duration = m_frameEnd - m_frameStart;

        m_renderSecs[m_avgIndex] = duration.count();
        m_fpsSecs[m_avgIndex] = elapsedSecs;

        m_avgIndex = (m_avgIndex + 1) % FPS_FRAMES;

        if (m_avgIndex == 0)
        {
            updateAvg();
        }
    }

    void FpsCounter::updateAvg()
    {
        float fpsTotal = 0.f;
        float renderTotal = 0.f;

        for (int i = 0; i < FPS_FRAMES; i++) {
            fpsTotal += m_fpsSecs[i];
            renderTotal += m_renderSecs[i];
        }

        m_fpsAvg = fpsTotal > 0 ? (float)FPS_FRAMES / fpsTotal : 0.f;
        m_millisAvg = renderTotal * 1000.f / (float)FPS_FRAMES;
    }

    std::string FpsCounter::formatSummary(const char* title) const noexcept
    {
        sprintf_s(
            m_formatBuffer,
            sizeof(m_formatBuffer),
            "%s - FPS: %-5.0f - RENDER: %-6.1fms",
            title,
            round(m_fpsAvg),
            m_millisAvg);
        return m_formatBuffer;
    }
}
