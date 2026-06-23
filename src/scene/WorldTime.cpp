#include "WorldTime.h"

WorldTime::WorldTime() = default;
WorldTime::~WorldTime() = default;

void WorldTime::setTimeSecs(double timeSecs) noexcept
{
    m_realElapsedSecs = (timeSecs - m_baseSecs) / m_timeScale;
    updateTime();
}

void WorldTime::setBaseSecs(double baseSecs) noexcept
{
    m_baseSecs = baseSecs;
    updateTime();
}

void WorldTime::updateRealElapsed(double elapsedSecs) noexcept
{
    m_realElapsedSecs += elapsedSecs;
    updateTime();
}

void WorldTime::setTimeScale(double scale) noexcept
{
    m_timeScale = scale;
    updateTime();
}

void WorldTime::updateTime() noexcept
{
    m_timeSecs = m_baseSecs + m_realElapsedSecs * m_timeScale;
}
