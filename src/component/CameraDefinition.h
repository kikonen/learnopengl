#pragma once

#include <vector>
#include <array>

#include <glm/glm.hpp>

#include "CameraType.h"

struct CameraDefinition {
    CameraType m_type{ CameraType::fps };

    bool m_default{ false };

    bool m_orthogonal{ false };
    std::array<float, 4> m_viewport{ 0.f };

    float m_fov{ 45.f };

    glm::vec3 m_front{ 0.f, 0.f, -1.f };
    glm::vec3 m_up{ 0.f, 1.f, 0.f };

    glm::vec3 m_offset{ 40.f, 0.f, 0.f };

    float m_pitch{ 0.f };
    float m_pitchSpeed{ 0.f };
    float m_yawSpeed{ 0.f };

    // [horiz_offset, vert_offset, target_dist
    glm::vec3 m_distance{ 0.f };

    // 64
    float m_springConstant{ 1024.f };

    std::vector<glm::vec3> m_path;
    float m_speed{ 0.f };
};
