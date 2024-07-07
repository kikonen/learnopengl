#pragma once

#include <vector>

#include "Clip.h"

namespace animation {
    struct Metadata {
        Metadata();
        ~Metadata();

        std::vector<animation::Clip> m_clips;
    };
}
