#pragma once

#include <atomic>
#include <string>

#include <glm/glm.hpp>

#include "util/Ref.h"

#include "WorldTime.h"

namespace loader {
    struct WorldData;
}

// Per-scene simulation world: owns the authoritative WorldTime (advanced on the
// worker thread) plus the day-night sun model.
//
// Sun direction / color / sky-blend are PURE functions of an absolute world time,
// so the render thread can derive them from the published atomic time without ever
// touching WT-mutated state.
//
// Threading contract:
//  - updateWT() is the ONLY mutator of the clock; call it from the worker thread.
//  - config fields are set once at load (before the WT loop spins) and treated as
//    read-only during the run, so RT may read them freely.
//  - RT reads publishedTimeSecs() / getSkyBlend() (atomics) and the const config,
//    then derives sun dir/color via the pure functions.
//  - RT->WT mutations (scrub time, change scale) must route via invokeLaterWT.
class World final : public util::RefCounted<>
{
public:
    World();
    ~World();

    void configure(const loader::WorldData& data) noexcept;

    WorldTime& getTime() noexcept { return m_time; }
    const WorldTime& getTime() const noexcept { return m_time; }

    // WT: advance the clock by real elapsed seconds, then republish derived values.
    void updateWT(double realElapsedSecs) noexcept;

    // --- published for RT (read once per frame) ---
    double publishedTimeSecs() const noexcept
    {
        return m_pubTimeSecs.load(std::memory_order_acquire);
    }

    // 0 = full day, 1 = full night
    float getSkyBlend() const noexcept
    {
        return m_pubSkyBlend.load(std::memory_order_acquire);
    }

    // --- pure derivations from an absolute world time (callable on any thread) ---

    // seconds within the civil day [0, 86400)
    static double timeOfDaySecs(double worldSecs) noexcept;
    // fraction of the day [0, 1)
    static double dayFraction(double worldSecs) noexcept;

    // unit vector FROM scene origin TOWARD the sun (Y up, +X east, tilt -> +Z)
    glm::vec3 sunDirectionToSun(double worldSecs) const noexcept;
    // light travel direction (FROM sun toward scene) == -sunDirectionToSun
    glm::vec3 sunLightDir(double worldSecs) const noexcept;
    float sunElevationDeg(double worldSecs) const noexcept;

    // dir-light color: bright white near noon, warm at sunrise/sunset, cold at night
    glm::vec3 sunColor(double worldSecs) const noexcept;
    // 0 = full day .. 1 = full night, smoothed across the twilight band
    float skyBlend(double worldSecs) const noexcept;

    // "YYYY-MM-DD HH:MM" of an absolute world time
    static std::string formatClock(double worldSecs);

public:
    // --- config (load-time; read-only during run) ---
    int   m_sunUpSecs{ 6 * 3600 };
    int   m_sunDownSecs{ 18 * 3600 };
    float m_sunTiltDeg{ 25.f };
    float m_sunTwilightDeg{ 10.f };

    glm::vec3 m_dayColor{ 1.0f, 0.98f, 0.95f };    // midday bright white
    glm::vec3 m_duskColor{ 1.0f, 0.55f, 0.25f };   // warm sunrise / sunset
    glm::vec3 m_nightColor{ 0.35f, 0.45f, 0.70f }; // cold moonlight

private:
    WorldTime m_time;

    std::atomic<double> m_pubTimeSecs{ 0. };
    std::atomic<float>  m_pubSkyBlend{ 0.f };
};
