#pragma once

#include <glm/glm.hpp>

#include <AL/al.h>

#include "size.h"

namespace audio
{
    struct Listener {
        Listener();
        ~Listener();

        void prepare();

        void update();
        void updatePos();

        audio::listener_id m_id;

        float m_gain{ 1.f };

        glm::vec3 m_pos{ 0.f };
        glm::vec3 m_vel{ 0.f };

        glm::vec3 m_front{ 0.f, 0.f, -1.f };
        glm::vec3 m_up{ 0.f, 1.f, 0.f };
    };
}
