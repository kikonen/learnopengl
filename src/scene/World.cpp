#include "World.h"

#include <cmath>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <fmt/format.h>

#include "loader/WorldData.h"

namespace {
    constexpr double DAY_SECS = 24.0 * 60.0 * 60.0;
}

World::World() = default;
World::~World() = default;

void World::configure(const loader::WorldData& data) noexcept
{
    m_time.setTimeScale(data.timeScale);
    m_time.setBaseSecs(data.timeBaseSecs);

    m_sunUpSecs = data.sunUpSecs;
    m_sunDownSecs = data.sunDownSecs;
    m_sunTiltDeg = data.sunAngle;
    m_sunTwilightDeg = data.sunTwilightAngle;

    m_dayColor = data.dayColor;
    m_duskColor = data.duskColor;
    m_nightColor = data.nightColor;

    // seed published values so RT has a sane state before the first WT advance
    const double t = m_time.getTimeSecs();
    m_pubTimeSecs.store(t, std::memory_order_release);
    m_pubSkyBlend.store(skyBlend(t), std::memory_order_release);
}

void World::updateWT(double realElapsedSecs) noexcept
{
    m_time.updateRealElapsed(realElapsedSecs);

    const double t = m_time.getTimeSecs();
    m_pubTimeSecs.store(t, std::memory_order_release);
    m_pubSkyBlend.store(skyBlend(t), std::memory_order_release);
}

double World::timeOfDaySecs(double worldSecs) noexcept
{
    // floored modulo so it stays in [0, DAY_SECS) even for negative times (year < 1970)
    return worldSecs - std::floor(worldSecs / DAY_SECS) * DAY_SECS;
}

double World::dayFraction(double worldSecs) noexcept
{
    return timeOfDaySecs(worldSecs) / DAY_SECS;
}

glm::vec3 World::sunDirectionToSun(double worldSecs) const noexcept
{
    // hour angle: 0 at solar noon (midpoint of up/down), +-PI at solar midnight
    const double noonFrac = (static_cast<double>(m_sunUpSecs) + static_cast<double>(m_sunDownSecs))
        * 0.5 / DAY_SECS;
    const double ha = (dayFraction(worldSecs) - noonFrac) * glm::two_pi<double>();
    const double tilt = glm::radians(static_cast<double>(m_sunTiltDeg));

    // base east-west arc (rotate up-vector around Z), then tilt the arc plane around X
    const glm::dvec3 toSun{
        -std::sin(ha),
         std::cos(ha) * std::cos(tilt),
         std::cos(ha) * std::sin(tilt)
    };
    return glm::normalize(glm::vec3(toSun));
}

glm::vec3 World::sunLightDir(double worldSecs) const noexcept
{
    return -sunDirectionToSun(worldSecs);
}

float World::sunElevationDeg(double worldSecs) const noexcept
{
    return glm::degrees(std::asin(glm::clamp(sunDirectionToSun(worldSecs).y, -1.f, 1.f)));
}

glm::vec3 World::sunColor(double worldSecs) const noexcept
{
    const float el = sunElevationDeg(worldSecs);
    const float tw = glm::max(m_sunTwilightDeg, 0.01f);

    // base: cold night below horizon -> bright day above
    const float dayW = glm::smoothstep(0.f, tw, el);
    const glm::vec3 base = glm::mix(m_nightColor, m_dayColor, dayW);

    // warm tint peaks at the horizon (el == 0), fades over the twilight band
    const float warm = 1.f - glm::clamp(std::abs(el) / tw, 0.f, 1.f);
    return glm::mix(base, m_duskColor, warm * 0.85f);
}

float World::skyBlend(double worldSecs) const noexcept
{
    const float el = sunElevationDeg(worldSecs);
    const float tw = glm::max(m_sunTwilightDeg, 0.01f);
    // el > +tw -> 0 (day), el < -tw -> 1 (night)
    return 1.f - glm::smoothstep(-tw, tw, el);
}

std::string World::formatClock(double worldSecs)
{
    using namespace std::chrono;

    const sys_seconds tp{ seconds{ static_cast<long long>(std::llround(worldSecs)) } };
    const auto dp = floor<days>(tp);
    const year_month_day ymd{ dp };
    const hh_mm_ss hms{ tp - dp };

    return fmt::format("{:04}-{:02}-{:02} {:02}:{:02}",
        static_cast<int>(ymd.year()),
        static_cast<unsigned>(ymd.month()),
        static_cast<unsigned>(ymd.day()),
        hms.hours().count(),
        hms.minutes().count());
}
