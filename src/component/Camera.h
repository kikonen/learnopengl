#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/size.h"

#include "kigl/kigl.h"

#include "asset/Frustum.h"

struct UpdateContext;
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

    ~Camera();

    void updateRT(const UpdateContext& ctx, Node& node) noexcept;

    bool isOrthagonal() const noexcept {
        return m_orthagonal;
    }

    const std::array<float, 4>& getViewport() const noexcept {
        return m_viewport;
    }

    // left, right, bottom, top
    void setViewport(
        const std::array<float, 4>& viewport);

    inline void setDefault(bool value) { m_default = value; }
    inline bool isDefault() const noexcept { return m_default; }

    inline void setEnabled(bool value) { m_enabled = value; }
    inline bool isEnabled() const noexcept { return m_enabled; }

    float getNearPlane() const noexcept { return m_nearPlane; }
    float getFarPlane() const noexcept { return m_farPlane; }

    // Setup projection
    // updates projection and projected matrices as needed
    void setupProjection(
        float aspectRatio,
        float nearPlane,
        float farPlane);

    const glm::mat4& getProjection() const noexcept;
    const glm::mat4& getProjected() noexcept;
    const glm::mat4& getView() noexcept;

    inline const ki::level_id getProjectionLevel() const noexcept { return m_projectionLevel; }
    inline const ki::level_id getProjectedLevel() const noexcept { return m_projectedLevel; }
    inline const ki::level_id getViewLevel() const noexcept { return m_viewLevel; }

    inline const glm::vec3& getWorldPosition() const noexcept {
        if (m_dirty) updateCamera();
        return m_worldPosition;
    }

    // NOTE KI for standalone camera
    void setWorldPosition(const glm::vec3& pos) noexcept;

    const glm::vec3& getViewFront() const noexcept
    {
        if (m_dirty) updateCamera();
        return m_viewFront;
    }

    const glm::vec3& getViewRight() const noexcept
    {
        if (m_dirty) updateCamera();
        return m_viewRight;
    }

    const glm::vec3& getViewUp() const noexcept
    {
        if (m_dirty) updateCamera();
        return m_viewUp;
    }

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

    void setDegreesRotation(const glm::vec3& rotation) noexcept;
    inline const glm::vec3& getDegreesRotation() const noexcept {
        return m_degreesRotation;
    }

    void updateCamera() const noexcept;

    const std::array<glm::vec4, 6> getFrustumPlanes() {
        return getFrustum().getPlanes();
    }

    inline const Frustum& getFrustum() const noexcept
    {
        if (m_dirtyFrustum) updateFrustum();
        return m_frustum;
    }

    void updateFrustum() const noexcept;

private:
    void updateFov(float fov) noexcept;

    void updateOrthagonalFrustum() const noexcept;
    void updatePerspectiveFrustum() const noexcept;

private:
    bool m_orthagonal{ false };
    // left, right, bottom, top
    std::array<float, 4> m_viewport{ 0.f };

    bool m_enabled = false;
    bool m_default = false;

    // NOTE KI *identity* matrix for standalone camera
    glm::quat m_nodeQuat{ 1.f, 0.f, 0.f, 0.f };
    ki::level_id m_nodeLevel{ 0 };

    float m_fov{ 45.f };
    float m_fovProjection = -1.0f;

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

    ki::level_id m_projectionLevel{ 0 };
    ki::level_id m_projectedLevel{ 0 };
    ki::level_id m_viewLevel{ 0 };

    glm::vec3 m_worldPosition{ 0.f };
    mutable glm::vec3 m_viewFront{ 0.f };
    mutable glm::vec3 m_viewRight{ 0.f };
    mutable glm::vec3 m_viewUp{ 0.f };

    //m_yaw = rotation.y;
    //m_pitch = rotation.x;
    //m_roll = rotation.z;
    glm::vec3 m_degreesRotation{ 0.f };

    mutable Frustum m_frustum;

    mutable bool m_dirty = true;
    mutable bool m_dirtyView = true;
    mutable bool m_dirtyProjected = true;
    mutable bool m_dirtyFrustum = true;
};
