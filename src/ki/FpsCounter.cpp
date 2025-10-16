#include "FpsCounter.h"

#include <fmt/format.h>

namespace ki {
    FpsCounter::FpsCounter()
    {
        m_frameStart = std::chrono::system_clock::now();
        m_frameEnd = m_frameStart;
    }

    void FpsCounter::startRender()
    {
        m_renderStart = std::chrono::system_clock::now();
    }

    void FpsCounter::endRender()
    {
        m_renderEnd = std::chrono::system_clock::now();

        std::chrono::duration<float> duration = m_renderEnd - m_renderStart;
        m_renderSecs[m_avgIndex] = duration.count();
    }

    void FpsCounter::startFrame()
    {
        //m_frameStart = std::chrono::system_clock::now();
        m_frameStart = m_frameEnd;
    }

    void FpsCounter::endFrame()
    {
        m_frameEnd = std::chrono::system_clock::now();
        std::chrono::duration<float> duration = m_frameEnd - m_frameStart;
        m_fpsSecs[m_avgIndex] = duration.count();

        m_avgIndex = (m_avgIndex + 1) % AVG_FRAMES;

        if (m_avgIndex == 0)
        {
            updateAvg();
        }
    }

    void FpsCounter::updateAvg()
    {
        float fpsTotal = 0.f;
        float renderTotal = 0.f;

        for (int i = 0; i < AVG_FRAMES; i++) {
            fpsTotal += m_fpsSecs[i];
            renderTotal += m_renderSecs[i];
        }

        m_fpsAvg = fpsTotal > 0 ? (float)AVG_FRAMES / fpsTotal : 0.f;
        m_renderAvg = renderTotal * 1000.f / (float)AVG_FRAMES;
    }

    std::string FpsCounter::formatSummary(std::string_view title) const noexcept
    {
        return fmt::format(
#ifdef _DEBUG
            "Debug {} - FPS: {} - RENDER: {:.3}ms",
#else
            "Release {} - FPS: {} - RENDER: {:.3}ms",
#endif
            title,
            round(m_fpsAvg),
            m_renderAvg);
    }
}
