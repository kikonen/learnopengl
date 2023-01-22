#pragma once

#include <chrono>

namespace ki {
    struct RenderClock {
        unsigned long frameCount = 0;
        //     std::chrono::system_clock::time_point ts;
        double ts = 0.0;
        float elapsedSecs = 0.0;
    };
}
