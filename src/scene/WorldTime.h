#pragma once

class WorldTime final
{
public:
    WorldTime();
    ~WorldTime();

    double getTimeSecs() const noexcept
    {
        return m_timeSecs;
    }

    void setTimeSecs(double worldTime) noexcept;

    double getBaseSecs() const noexcept
    {
        return m_baseSecs;
    }

    void setBaseSecs(double baseSecs) noexcept;

    void updateRealElapsed(double elapsedSecs) noexcept;

    double getTimeScale() const noexcept
    {
        return m_timeScale;
    }

    void setTimeScale(double scale) noexcept;

private:
    void updateTime() noexcept;

private:
    double m_realElapsedSecs{ 0. };

    double m_timeSecs{ 0. };
    double m_baseSecs{ 0. };
    double m_timeScale{ 100. };
};
