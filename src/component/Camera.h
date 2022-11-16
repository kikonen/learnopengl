#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "asset/Frustum.h"

#include "ki/GL.h"
#include "gui/Input.h"

struct CameraProjection {
};

/**
* https://learnopengl.com/Getting-started/Camera
*/
class Camera final
{
public:
    Camera(const glm::vec3& pos, const glm::vec3 front, const glm::vec3 aUp);
    ~Camera();

    // Setup projection
    // updates projection and projected matrices as needed
    void setupProjection(
        float aspectRatio,
        float nearPlane,
        float farPlane);

    const glm::mat4& getProjection() const noexcept;
    const glm::mat4& getProjected() noexcept;
    const glm::mat4& getView() noexcept;

    const int getProjectionLevel() const noexcept { return m_projectionLevel; }
    const int getProjectedLevel() const noexcept { return m_projectedLevel; }
    const int getViewLevel() const noexcept { return m_viewLevel; }

    const glm::vec3& getViewFront() noexcept;
    const glm::vec3& getViewRight() noexcept;
    const glm::vec3& getViewUp() noexcept;

    const Frustum& getFrustum() noexcept;

    const glm::vec3& getFront() noexcept;
    const glm::vec3& getRight() noexcept;
    const glm::vec3& getUp() noexcept;

    float getZoom() noexcept;
    void setZoom(float zoom) noexcept;

    void setPos(const glm::vec3& pos) noexcept;
    const glm::vec3& getPos() const noexcept;

    void setRotation(const glm::vec3& rotation) noexcept;
    const glm::vec3 getRotation() noexcept;

    void onKey(Input* input, const ki::RenderClock& clock) noexcept;
    void onMouseMove(Input* input, double xoffset, double yoffset) noexcept;
    void onMouseScroll(Input* input, double xoffset, double yoffset) noexcept;

private:
    void updateZoom(float aZoom) noexcept;
    void updateCamera() noexcept;
    void updateFrustum() noexcept;

private:
    float m_zoom = 45.0f;
    float m_zoomProjection = -1.0f;

    float m_moveStep = 10.0f;
    float m_rotateStep = 30.f;
    float m_zoomStep = 20.0f;
    float m_mouseSensitivity = 0.1f;

    glm::vec3 m_pos;
    glm::vec3 m_front;
    glm::vec3 m_right;
    glm::vec3 m_up;

    float m_aspectRatio = -1;
    float m_nearPlane = -1;
    float m_farPlane = -1;

    glm::mat4 m_projectionMatrix;
    mutable glm::mat4 m_projectedMatrix;

    glm::mat4 m_viewMatrix;
    glm::mat4 m_rotateMat;

    int m_projectionLevel = -1;
    int m_projectedLevel = -1;
    int m_viewLevel = -1;

    glm::vec3 m_viewFront;
    glm::vec3 m_viewRight;
    glm::vec3 m_viewUp;

    float m_yaw = 0;
    float m_pitch = 0;
    float m_roll = 0;

    bool m_dirty = true;
    bool m_dirtyProjected = true;
    bool m_dirtyFrustum = true;
};

