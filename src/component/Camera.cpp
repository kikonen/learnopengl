#include "Camera.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glm/ext.hpp>


const float MIN_ZOOM = 1.0f;
const float MAX_ZOOM = 45.0f;


Camera::Camera(const glm::vec3& aPos, const glm::vec3 aFront, const glm::vec3 aUp)
{
    m_pos = aPos;

    // Default: look to Z direction
    m_front = glm::normalize(aFront);
    m_up = glm::normalize(aUp);
    m_right = glm::normalize(glm::cross(m_front, m_up));

    m_rotateMat = glm::mat4(1.0f);

    m_dirty = true;
}

Camera::~Camera()
{
}

void Camera::setupProjection(
    float aspectRatio,
    float nearPlane,
    float farPlane)
{
    if (m_aspectRatio == aspectRatio &&
        m_nearPlane == nearPlane &&
        m_farPlane == farPlane &&
        m_zoomProjection == m_zoom) return;

    m_aspectRatio = aspectRatio;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_zoomProjection = m_zoom;

    m_projectionMatrix = glm::perspective(
        glm::radians(m_zoomProjection),
        m_aspectRatio,
        m_nearPlane,
        m_farPlane);

    m_projectionLevel++;
    m_dirtyProjected = true;
}

const glm::mat4& Camera::getProjection() const noexcept
{
    return m_projectionMatrix;
}

const glm::mat4& Camera::getProjected() noexcept
{
    if (!m_dirtyProjected) return m_projectedMatrix;
    m_projectedMatrix = m_projectionMatrix * getView();
    m_dirtyProjected = false;
    m_projectedLevel++;
    return m_projectedMatrix;
}

const glm::mat4& Camera::getView() noexcept
{
    if (!m_dirty) return m_viewMatrix;

    updateCamera();
    m_viewMatrix = glm::lookAt(
        m_pos,
        m_pos + m_viewFront,
        m_viewUp);
    return m_viewMatrix;
}

const glm::vec3& Camera::getViewFront() noexcept
{
    if (m_dirty) updateCamera();
    return m_viewFront;
}

const glm::vec3& Camera::getViewRight() noexcept
{
    if (m_dirty) updateCamera();
    return m_viewRight;
}

const glm::vec3& Camera::getViewUp() noexcept
{
    if (m_dirty) updateCamera();
    return m_viewUp;
}

void Camera::setFront(const glm::vec3& front) noexcept
{
    if (m_front != front) {
        m_dirty = true;
        m_front = front;
    }
}

const glm::vec3& Camera::getFront() noexcept
{
    return m_front;
}

const glm::vec3& Camera::getRight() noexcept
{
    return m_right;
}

void Camera::setUp(const glm::vec3& up) noexcept
{
    if (m_up != up) {
        m_dirty = true;
        m_up = up;
    }
}

const glm::vec3& Camera::getUp() noexcept
{
    return m_up;
}

float Camera::getZoom() noexcept
{
    return m_zoom;
}

void Camera::setZoom(float zoom) noexcept
{
    updateZoom(zoom);
}

void Camera::setPos(const glm::vec3& pos) noexcept
{
    if (m_pos != pos) {
        m_pos = pos;
        m_dirty = true;
    }
}

const glm::vec3& Camera::getPos() const noexcept
{
    return m_pos;
}

void Camera::setRotation(const glm::vec3& rotation) noexcept
{
    if (m_yaw != rotation.y ||
        m_pitch != rotation.x ||
        m_roll != rotation.z)
    {
        m_yaw = rotation.y;
        m_pitch = rotation.x;
        m_roll = rotation.z;
        m_dirty = true;
    }
}

const glm::vec3 Camera::getRotation() noexcept
{
    return glm::vec3(m_pitch, m_yaw, m_roll);
}

void Camera::onKey(Input* input, const ki::RenderClock& clock) noexcept
{
    float dt = clock.elapsedSecs;
    float moveSize = m_moveStep;
    float rotateSize = m_rotateStep;
    if (input->isModifierDown(Modifier::SHIFT)) {
        moveSize *= 2;
        rotateSize *= 2;
    }

    if (input->isKeyDown(Key::FORWARD)) {
        updateCamera();
        m_pos += m_viewFront * dt * moveSize;
        m_dirty = true;
    }

    if (input->isKeyDown(Key::BACKWARD)) {
        updateCamera();
        m_pos -= m_viewFront * dt * moveSize;
        m_dirty = true;
    }

    if (input->isKeyDown(Key::LEFT)) {
        updateCamera();
        m_pos -= m_viewRight * dt * moveSize;
        m_dirty = true;
    }

    if (input->isKeyDown(Key::RIGHT)) {
        updateCamera();
        m_pos += m_viewRight * dt * moveSize;
        m_dirty = true;
    }

    if (input->isKeyDown(Key::UP)) {
        updateCamera();
        m_pos += m_viewUp * dt * moveSize;
        m_dirty = true;
    }

    if (input->isKeyDown(Key::DOWN)) {
        updateCamera();
        m_pos -= m_viewUp * dt * moveSize;
        m_dirty = true;
    }

    if (true) {
        if (input->isKeyDown(Key::ROTATE_LEFT)) {
            m_yaw += rotateSize * dt;
            m_dirty = true;
        }
        if (input->isKeyDown(Key::ROTATE_RIGHT)) {
            m_yaw -= rotateSize * dt;
            m_dirty = true;
        }
    }

    if (input->isKeyDown(Key::ZOOM_IN)) {
        updateZoom(m_zoom - m_zoomStep * dt);
    }
    if (input->isKeyDown(Key::ZOOM_OUT)) {
        updateZoom(m_zoom + m_zoomStep * dt);
    }
}

void Camera::onMouseMove(Input* input, double xoffset, double yoffset) noexcept
{
    bool changed = false;
    const float MAX_ANGLE = 89.f;

    if (true) {
        m_yaw -= m_mouseSensitivity * xoffset;
        changed = true;
    }

    if (true) {
        m_pitch += m_mouseSensitivity * yoffset;
        changed = true;

        if (m_pitch < -MAX_ANGLE) {
            m_pitch = -MAX_ANGLE;
        }
        if (m_pitch > MAX_ANGLE) {
            m_pitch = MAX_ANGLE;
        }
    }

    if (changed) {
        m_dirty = true;
    }
}

void Camera::onMouseScroll(Input* input, double xoffset, double yoffset) noexcept
{
    updateZoom(m_zoom - yoffset);
}

void Camera::updateZoom(float zoom) noexcept
{
    if (zoom < MIN_ZOOM) {
        zoom = MIN_ZOOM;
    }
    if (zoom > MAX_ZOOM) {
        zoom = MAX_ZOOM;
    }
    if (m_zoom != zoom) {
        m_zoom = zoom;
        m_dirty = true;
    }
}

void Camera::updateCamera() noexcept
{
    if (!m_dirty) return;
    m_dirty = false;

    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    m_rotateMat = glm::toMat4(glm::quat(glm::radians(glm::vec3(m_pitch, m_yaw, m_roll))));

    // NOTE KI glm::normalize for vec4 *IS* incorrect (4d len...)
    m_viewFront = glm::normalize(glm::vec3(m_rotateMat * glm::vec4(m_front, 1.f)));
    m_viewUp = glm::normalize(glm::vec3(m_rotateMat * glm::vec4(m_up, 1.f)));
    m_viewRight = glm::normalize(glm::cross(m_viewFront, m_viewUp));

    m_dirtyProjected = true;
    m_viewLevel++;
}
