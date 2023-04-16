#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"

#include "asset/Frustum.h"

class UpdateContext;
class Node;

struct CameraProjection {
};

/**
* https://learnopengl.com/Getting-started/Camera
*/
class Camera final
{
public:
    Camera();

    Camera(
        const glm::vec3& pos,
        const glm::vec3 front,
        const glm::vec3 up);
    ~Camera() = default;

    void update(const UpdateContext& ctx, Node& node) noexcept;

    inline void setDefault(bool value) { m_default = value; }
    inline bool isDefault() const { return m_default; }

    inline void setEnabled(bool value) { m_enabled = value; }
    inline bool isEnabled() const { return m_enabled; }

    // Setup projection
    // updates projection and projected matrices as needed
    void setupProjection(
        float aspectRatio,
        float nearPlane,
        float farPlane);

    const glm::mat4& getProjection() const noexcept;
    const glm::mat4& getProjected() noexcept;
    const glm::mat4& getView() noexcept;

    inline const int getProjectionLevel() const noexcept { return m_projectionLevel; }
    inline const int getProjectedLevel() const noexcept { return m_projectedLevel; }
    inline const int getViewLevel() const noexcept { return m_viewLevel; }

    inline const glm::vec3& getWorldPosition() const noexcept { return m_worldPosition;  }

    // NOTE KI for standalone camera
    void setWorldPosition(const glm::vec3& pos) noexcept;

    const glm::vec3& getViewFront() const noexcept;
    const glm::vec3& getViewRight() const noexcept;
    const glm::vec3& getViewUp() const noexcept;

    void setAxis(
        const glm::vec3& front,
        const glm::vec3& up) noexcept;

    inline const glm::vec3& getFront() const noexcept {
        return m_front;
    }

    //inline const glm::vec3& getRight() const noexcept {
    //    return m_right;
    //}

    inline const glm::vec3& getUp() const noexcept {
        return m_up;
    }

    inline float getFov() const noexcept {
        return m_fov;
    }

    void setFov(float fov) noexcept;
    void adjustFov(float adjustement) noexcept;

    // NOTE KI for node camera; relative to node
    void setPosition(const glm::vec3& pos) noexcept;

    // NOTE KI for node camera; relative to node
    inline const glm::vec3& getPosition() const noexcept {
        return m_position;
    }

    void setRotation(const glm::vec3& rotation) noexcept;
    inline const glm::vec3& getRotation() const noexcept {
        return m_rotation;
    }

    void updateCamera() const noexcept;

    const std::array<glm::vec4, 6> getFrustumPlanes() {
        return getFrustum().getPlanes();
    }

    const Frustum& getFrustum() const noexcept
    {
        updateCamera();
        if (m_dirtyFrustum) updateFrustum();
        return m_frustum;
    }

    void updateFrustum() const noexcept;

private:
    void updateFov(float fov) noexcept;

private:
    bool m_enabled = false;
    bool m_default = false;

    // NOTE KI *identity* matrix for standalone camera
    glm::mat4 m_nodeModelMatrix{ 1.f };
    int m_nodeLevel = -1;

    float m_fov{ 45.f };
    float m_fovProjection = -1.0f;

    glm::vec3 m_position{ 0.f };

    // *DIRECTION* at camera is pointing at (== target)
    // *NOT* required to be orthogonal to up
    glm::vec3 m_front{ 0.f, 0.f, -1.f };

    // *UP* direction of world
    glm::vec3 m_up{ 0.f, 1.f, 0.f };

    //// *RIGHT* cross product of front * up
    //glm::vec3 m_right{ 1.f, 0.f, 0.f };

    float m_aspectRatio = -1;
    float m_nearPlane = -1;
    float m_farPlane = -1;

    glm::mat4 m_projectionMatrix{ 1.f };
    mutable glm::mat4 m_projectedMatrix{ 1.f };

    mutable glm::mat4 m_viewMatrix{ 1.f };
    mutable glm::mat4 m_rotateMatrix{ 1.f };

    int m_projectionLevel = -1;
    int m_projectedLevel = -1;
    int m_viewLevel = -1;

    glm::vec3 m_worldPosition{ 0.f };
    mutable glm::vec3 m_viewFront{ 0.f };
    mutable glm::vec3 m_viewRight{ 0.f };
    mutable glm::vec3 m_viewUp{ 0.f };

    //m_yaw = rotation.y;
    //m_pitch = rotation.x;
    //m_roll = rotation.z;
    glm::vec3 m_rotation{ 0.f };

    mutable Frustum m_frustum;

    mutable bool m_dirty = true;
    mutable bool m_dirtyView = true;
    mutable bool m_dirtyProjected = true;
    mutable bool m_dirtyFrustum = true;
};
