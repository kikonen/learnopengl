#pragma once

#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace util
{
    // Primary axis convention for mesh/physics shapes
    // Determines which axis is "up" or "length" direction
    enum class Axis : uint8_t {
        y,       // +Y up (OpenGL, Jolt default)
        z,       // +Z up (Blender, generator lib)
        x,       // +X up
        neg_y,   // -Y up
        neg_z,   // -Z up
        neg_x,   // -X up
    };

    // Front direction convention for meshes
    // Determines which direction the mesh is facing
    enum class Front : uint8_t {
        z,       // Facing +Z (default)
        x,       // Facing +X
        neg_x,   // Facing -X
        neg_z,   // Facing -Z
    };

    // Convert axis convention to rotation quaternion
    // Returns rotation needed to align the given axis to +Y
    inline glm::quat axisToRotation(Axis axis) noexcept
    {
        switch (axis) {
        case Axis::z:
            // Rotate -90 degrees around X to bring +Z to +Y
            return glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
        case Axis::x:
            // Rotate +90 degrees around Z to bring +X to +Y
            return glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
        case Axis::neg_y:
            // Rotate 180 degrees around X to bring -Y to +Y
            return glm::angleAxis(glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f));
        case Axis::neg_z:
            // Rotate +90 degrees around X to bring -Z to +Y
            return glm::angleAxis(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
        case Axis::neg_x:
            // Rotate -90 degrees around Z to bring -X to +Y
            return glm::angleAxis(glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f));
        case Axis::y:
        default:
            return glm::quat{ 1.f, 0.f, 0.f, 0.f };
        }
    }

    // Convert front direction to rotation quaternion
    // Returns rotation around Y-axis to align the given front to +Z
    inline glm::quat frontToRotation(Front front) noexcept
    {
        switch (front) {
        case Front::x:
            // Facing +X, rotate -90 degrees around Y to face +Z
            return glm::angleAxis(glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));
        case Front::neg_x:
            // Facing -X, rotate +90 degrees around Y to face +Z
            return glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
        case Front::neg_z:
            // Facing -Z, rotate 180 degrees around Y to face +Z
            return glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
        case Front::z:
        default:
            return glm::quat{ 1.f, 0.f, 0.f, 0.f };
        }
    }

    // Inverse rotation: from Y-axis back to the original axis
    inline glm::quat axisToInvRotation(Axis axis) noexcept
    {
        return glm::conjugate(axisToRotation(axis));
    }

    // Inverse rotation: from +Z front back to the original front
    inline glm::quat frontToInvRotation(Front front) noexcept
    {
        return glm::conjugate(frontToRotation(front));
    }
}
