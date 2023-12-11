#pragma once

#include <glm/glm.hpp>

#include "size.h"


namespace audio
{
    struct Listener {
        Listener();
        ~Listener();

        audio::listener_id m_id;

        glm::vec3 m_pos{ 0.f };
    };
}
