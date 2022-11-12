#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "asset/Frustum.h"

#include "ki/GL.h"
#include "gui/Input.h"

/**
* https://learnopengl.com/Getting-started/Camera
*/
class Camera final
{
public:
    Camera(const glm::vec3& pos, const glm::vec3 front, const glm::vec3 aUp);
    ~Camera();

    const glm::mat4& getView() noexcept;
    const glm::vec3& getViewFront() noexcept;
    const glm::vec3& getViewRight() noexcept;
    const glm::vec3& getViewUp() noexcept;

    const Frustum& getFrustum() noexcept;

    const glm::vec3& getFront() noexcept;
    const glm::vec3& getRight() noexcept;
    const glm::vec3& getUp() noexcept;

    double getZoom() noexcept;
    void setZoom(double zoom) noexcept;

    void setPos(const glm::vec3& pos) noexcept;
    const glm::vec3& getPos() const noexcept;

    void setRotation(const glm::vec3& rotation) noexcept;
    const glm::vec3 getRotation() noexcept;

    void onKey(Input* input, const ki::RenderClock& clock) noexcept;
    void onMouseMove(Input* input, double xoffset, double yoffset) noexcept;
    void onMouseScroll(Input* input, double xoffset, double yoffset) noexcept;

private:
    void updateZoom(double aZoom) noexcept;
    void updateCamera() noexcept;
    void updateFrustum() noexcept;

private:
    double m_zoom = 45.0f;

    float m_moveStep = 10.0f;
    float m_rotateStep = 30.f;
    float m_zoomStep = 20.0f;
    float m_mouseSensitivity = 0.1f;

    glm::vec3 m_pos;
    glm::vec3 m_front;
    glm::vec3 m_right;
    glm::vec3 m_up;

    glm::mat4 m_viewMat;
    glm::mat4 m_rotateMat;

    glm::vec3 m_viewFront;
    glm::vec3 m_viewRight;
    glm::vec3 m_viewUp;

    float m_yaw = 0;
    float m_pitch = 0;
    float m_roll = 0;

    bool m_dirty = true;
    bool m_dirtyFrustum = true;
};

