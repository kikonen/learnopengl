#pragma once

#include <chrono>

#include "fmt/format.h"

#include "util/Log.h"

namespace ki {
    struct Timer final
     {
        std::string label;
        std::chrono::time_point<std::chrono::steady_clock> start, end;
        std::chrono::duration<float> duration;

        Timer(std::string&& label) {
            this->label = label;
            start = std::chrono::high_resolution_clock::now();
        }

        ~Timer() {
            end = std::chrono::high_resolution_clock::now();
            duration = end - start;

            float ms = duration.count() * 1000.f;
            KI_INFO(fmt::format("{}: {}ms", label, ms));
        }
    };
}
