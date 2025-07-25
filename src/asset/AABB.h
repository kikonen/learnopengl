#pragma once

#include <glm/glm.hpp>

// https://bruop.github.io/frustum_culling/
struct AABB final
{
    AABB() noexcept = default;

    AABB(bool minmax)
        : AABB(
            glm::vec3(std::numeric_limits<float>::max()),
            glm::vec3(std::numeric_limits<float>::min()),
            false)
    {}

    AABB(const AABB& aabb) noexcept
        : AABB(aabb.m_min, aabb.m_max, aabb.m_quad)
    {}

    AABB& operator=(const AABB& aabb) noexcept
    {
        m_min = aabb.m_min;
        m_max = aabb.m_max;
        updateVolume();
        return *this;
    }

    AABB(
        const glm::vec3& min,
        const glm::vec3& max,
        bool quad) noexcept
        : m_min{ min },
        m_max{ max },
        m_quad{ quad }
    {
        updateVolume();
    }

    ~AABB() noexcept = default;

    void prepareMinMax() {
        m_volume = { 0.f, 0.f, 0.f, 0.f };
        m_min = glm::vec3(std::numeric_limits<float>::max());
        m_max = glm::vec3(std::numeric_limits<float>::min());
    }

    inline const glm::vec4& getVolume() const {
        return m_volume;
    }

    void updateVolume();

    void minmax(const glm::vec3& pos);
    void minmax(const AABB& o);

public:
    glm::vec4 m_volume{ 0.f };
    glm::vec3 m_min{ 0.f };
    glm::vec3 m_max{ 0.f };
    bool m_quad : 1 { false };
};
