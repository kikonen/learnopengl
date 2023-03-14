#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"

class Node;

struct CameraProjection {
};

/**
* https://learnopengl.com/Getting-started/Camera
*/
class Camera final
{
public:
    Camera(
        const glm::vec3& pos,
        const glm::vec3 front,
        const glm::vec3 up,
        bool nodeCamera = false);
    ~Camera();

    void update(Node& node);

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

    const glm::vec3& getViewFront() const noexcept;
    const glm::vec3& getViewRight() const noexcept;
    const glm::vec3& getViewUp() const noexcept;

    void setFront(const glm::vec3& front) noexcept;

    inline const glm::vec3& getFront() const noexcept {
        return m_front;
    }

    inline const glm::vec3& getRight() const noexcept {
        return m_right;
    }

    void setUp(const glm::vec3& up) noexcept;
    inline const glm::vec3& getUp() const noexcept {
        return m_up;
    }

    inline float getZoom() const noexcept {
        return m_zoom;
    }

    void setZoom(float zoom) noexcept;
    void adjustZoom(float adjustement) noexcept;

    void setPosition(const glm::vec3& pos) noexcept;
    inline const glm::vec3& getPosition() const noexcept {
        return m_position;
    }

    void setRotation(const glm::vec3& rotation) noexcept;
    inline const glm::vec3& getRotation() const noexcept {
        return m_rotation;
    }

    void updateCamera() const noexcept;

private:
    void updateZoom(float aZoom) noexcept;

private:
    bool m_enabled = false;
    bool m_default = false;

    bool m_nodeCamera = false;
    int m_nodeMatrixLevel = -1;

    float m_zoom = 45.0f;
    float m_zoomProjection = -1.0f;

    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_right;
    glm::vec3 m_up;

    float m_aspectRatio = -1;
    float m_nearPlane = -1;
    float m_farPlane = -1;

    glm::mat4 m_projectionMatrix;
    mutable glm::mat4 m_projectedMatrix;

    mutable glm::mat4 m_viewMatrix;
    mutable glm::mat4 m_rotateMatrix;

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
    //float m_yaw = 0;
    //float m_pitch = 0;
    //float m_roll = 0;

    mutable bool m_dirty = true;
    mutable bool m_dirtyView = true;
    mutable bool m_dirtyProjected = true;
};

