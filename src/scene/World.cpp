#include "World.h"

#include <cmath>
#include <chrono>
#include <format>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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

    // explicit axis wins; otherwise derive from tilt so the arc matches the
    // legacy east-west-with-tilt model exactly
    if (data.hasSunAxis) {
        m_sunAxis = glm::normalize(data.sunAxis);
    }
    else {
        const double tilt = glm::radians(static_cast<double>(m_sunTiltDeg));
        m_sunAxis = glm::vec3{ 0.f,
            static_cast<float>(-std::sin(tilt)),
            static_cast<float>(std::cos(tilt)) };
    }

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

    // The sun traces a circle whose plane normal is m_sunAxis. Build an orthonormal
    // basis (r1, r2) in that plane with r1 = the "most upward" direction, so ha=0
    // (noon) is the highest point of the arc.
    const glm::vec3 a = glm::normalize(m_sunAxis);
    const glm::vec3 up{ 0.f, 1.f, 0.f };

    glm::vec3 r1 = up - glm::dot(up, a) * a;
    if (glm::dot(r1, r1) < 1e-6f) {
        // axis ~ vertical: arc lies near the horizon plane, pick an arbitrary east
        const glm::vec3 east{ 1.f, 0.f, 0.f };
        r1 = east - glm::dot(east, a) * a;
    }
    r1 = glm::normalize(r1);
    const glm::vec3 r2 = glm::normalize(glm::cross(a, r1));

    const glm::vec3 toSun =
        static_cast<float>(std::cos(ha)) * r1 +
        static_cast<float>(std::sin(ha)) * r2;
    return glm::normalize(toSun);
}

glm::vec3 World::sunLightDir(double worldSecs) const noexcept
{
    return -sunDirectionToSun(worldSecs);
}

float World::sunElevationDeg(double worldSecs) const noexcept
{
    return glm::degrees(std::asin(glm::clamp(sunDirectionToSun(worldSecs).y, -1.f, 1.f)));
}

DirLightState World::primaryLight(double worldSecs) const noexcept
{
    const glm::vec3 sunToward = sunDirectionToSun(worldSecs);

    // pick the body above the horizon; moon is the anti-sun, so exactly one is up
    const bool sunUp = sunToward.y >= 0.f;
    const glm::vec3 toward = sunUp ? sunToward : -sunToward;

    const float el = glm::degrees(std::asin(glm::clamp(toward.y, -1.f, 1.f))); // >= 0
    const float tw = glm::max(m_sunTwilightDeg, 0.01f);

    // horizon fade: energy goes to ~0 at the horizon so the sun<->moon handover is
    // smooth and grazing light doesn't blast sideways. This is ENERGY (-> intensity),
    // kept out of the color so chroma and energy stay separate.
    const float w = glm::smoothstep(0.f, tw, el);

    glm::vec3 color;
    if (sunUp) {
        // warm (dusk) near the horizon -> bright white high in the sky (chroma only)
        color = glm::mix(m_duskColor, m_dayColor, w);
    }
    else {
        color = m_nightColor; // cold moonlight (chroma only)
    }

    DirLightState state;
    state.dir = glm::normalize(-toward); // travel dir, always downward (body is above horizon)
    state.color = color;
    state.weight = w;
    state.isMoon = !sunUp;
    return state;
}

float World::skyBlend(double worldSecs) const noexcept
{
    const float el = sunElevationDeg(worldSecs);
    const float tw = glm::max(m_sunTwilightDeg, 0.01f);
    // el > +tw -> 0 (day), el < -tw -> 1 (night)
    return 1.f - glm::smoothstep(-tw, tw, el);
}

std::string World::formatClock(double worldSecs, std::string_view fmtSpec)
{
    using namespace std::chrono;

    const sys_seconds tp{ seconds{ static_cast<long long>(std::llround(worldSecs)) } };

    // wrap the strftime-style spec into a std::format replacement field, e.g.
    // "%H:%M" -> "{:%H:%M}", and resolve at runtime
    const std::string fmtStr = "{:" + std::string{ fmtSpec } + "}";
    try {
        return std::vformat(fmtStr, std::make_format_args(tp));
    }
    catch (const std::format_error&) {
        // bad spec: surface it literally instead of throwing (callers are noexcept)
        return std::string{ fmtSpec };
    }
}
