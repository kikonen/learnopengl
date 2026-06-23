#pragma once

#include <glm/glm.hpp>

namespace loader
{
    struct WorldData
    {
        // true once a `world:` block was actually parsed; scenes without one get no
        // World (and thus a static day skybox / no day-night clock)
        bool loaded{ false };

        // initial world time: epoch seconds (Unix/proleptic-Gregorian), parsed from
        // an ISO8601 "time_base" string (e.g. "2026-06-23T12:00:00"). May be negative
        // for years < 1970. Defaults to noon (1970-01-01 12:00) so a world without an
        // explicit time_base starts in daylight rather than at midnight.
        double timeBaseSecs{ 12.0 * 60.0 * 60.0 };

        // sim seconds advanced per real second (1 = realtime, 0 = paused)
        double timeScale{ 100.0 };

        // daylight window as seconds-of-day, parsed from "HH:MM"
        int sunUpSecs{ 6 * 3600 };
        int sunDownSecs{ 18 * 3600 };

        // arc tilt (degrees) and twilight band half-width (degrees of elevation)
        float sunAngle{ 25.f };
        float sunTwilightAngle{ 10.f };

        // optional explicit daily-rotation axis; when unset it is derived from sunAngle
        bool hasSunAxis{ false };
        glm::vec3 sunAxis{ 0.f, 0.f, 1.f };

        // dir-light colors per phase
        glm::vec3 dayColor{ 1.0f, 0.98f, 0.95f };    // midday bright white
        glm::vec3 duskColor{ 1.0f, 0.55f, 0.25f };   // warm sunrise / sunset
        glm::vec3 nightColor{ 0.35f, 0.45f, 0.70f }; // cold moonlight
    };
}
