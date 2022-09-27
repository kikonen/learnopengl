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

    const glm::mat4& getView();
    const glm::vec3& getViewFront();
    const glm::vec3& getViewRight();
    const glm::vec3& getViewUp();

    const Frustum& getFrustum();

    const glm::vec3& getFront();
    const glm::vec3& getRight();
    const glm::vec3& getUp();

    double getZoom();
    void setZoom(double zoom);

    void setPos(const glm::vec3& pos);
    const glm::vec3& getPos() const;

    void setRotation(const glm::vec3& rotation);
    const glm::vec3 getRotation();

    void onKey(Input* input, const RenderClock& clock);
    void onMouseMove(Input* input, double xoffset, double yoffset);
    void onMouseScroll(Input* input, double xoffset, double yoffset);

private:
    void updateZoom(double aZoom);
    void updateCamera();
    void updateFrustum();

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

