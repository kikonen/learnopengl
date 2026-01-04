#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "ki/size.h"

#include "asset/Frustum.h"

// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
struct SphereVolume {
    float x{ 0.f };
    float y{ 0.f };
    float z{ 0.f };
    float radius{ 0.f };

    SphereVolume() noexcept = default;

    SphereVolume(float o) noexcept
        : x{ o },
        y{ o },
        z{ o },
        radius{ o }
    {}

    SphereVolume(float o_x, float o_y, float o_z, float o_w) noexcept
        : x{ o_x },
        y{ o_y },
        z{ o_z },
        radius{ o_w }
    {}

    SphereVolume(const glm::vec4& o) noexcept
        : x{ o.x },
        y{ o.y },
        z{ o.z },
        radius{ o.w }
    {}

    SphereVolume(const glm::vec3& o) noexcept
        : x{ o.x },
        y{ o.y },
        z{ o.z },
        radius{ 0.f }
    {}

    SphereVolume(const glm::vec3& o, float o_radius) noexcept
        : x{ o.x },
        y{ o.y },
        z{ o.z },
        radius{ o_radius }
    {}

    ~SphereVolume() noexcept = default;

    SphereVolume& operator=(const glm::vec4& o)
    {
        x = o.x;
        y = o.y;
        z = o.z;
        radius = o.w;
        return *this;
    }

    SphereVolume& operator=(const glm::vec3& o)
    {
        x = o.x;
        y = o.y;
        z = o.z;
        radius = 0.f;
        return *this;
    }

    bool operator==(const SphereVolume& o)
    {
        return x == o.x && y == o.y && z == o.z && radius == o.radius;
    }

    bool operator==(const glm::vec4& o)
    {
        return x == o.x && y == o.y && z == o.z && radius == o.w;
    }

    bool operator==(const glm::vec3& o)
    {
        return x == o.x && y == o.y && z == o.z && radius == 0.f;
    }

    float operator[](int i) const
    {
        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
        if (i == 3) return radius;
        return 0.f;
    }

    glm::vec4 toVec4() const noexcept
    {
        return { x, y, z, radius };
    }

    glm::vec3 getCenter() const noexcept
    {
        return { x, y, z };
    }

    glm::vec4 getPosition() const noexcept
    {
        return { x, y, z, 1.f };
    }

    float getRadius() const noexcept
    {
        return radius;
    }

    bool isOnFrustum(const Frustum& frustum) const noexcept;
};

