#pragma once

#include <glm/glm.hpp>

#include <AL/al.h>

#include "ki/size.h"

#include "size.h"

class Node;

namespace audio
{
    struct Listener {
        Listener();
        Listener(Listener&&) noexcept;
        ~Listener();

        inline bool isReady() const { return m_matrixLevel > -1; }

        void prepare();

        void updateFromNode();

        void update();
        void updatePos();

        audio::listener_id m_id{ 0 };

        bool m_default{ false };
        float m_gain{ 1.f };

        glm::vec3 m_pos{ 0.f };
        glm::vec3 m_vel{ 0.f };

        glm::vec3 m_front{ 0.f, 0.f, -1.f };
        glm::vec3 m_up{ 0.f, 1.f, 0.f };

        ki::level_id m_matrixLevel{ (ki::level_id)-1 };
        Node* m_node{ nullptr };
    };
}
