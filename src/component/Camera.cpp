#include "Camera.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glm/ext.hpp>

#include "model/Node.h"
#include "scene/RenderContext.h"

const float MIN_ZOOM = 10.0f;
// NOTE KI 90 to allow cubemap & shadowmap wide angle
const float MAX_ZOOM = 90.0f;


Camera::Camera(
    const glm::vec3& pos,
    const glm::vec3 front,
    const glm::vec3 up,
    bool nodeCamera)
{
    m_nodeCamera = nodeCamera;

    m_position = pos;
    m_worldPosition = pos;

    // Default: look to Z direction
    m_front = glm::normalize(front);
    m_up = glm::normalize(up);
    m_right = glm::normalize(glm::cross(m_front, m_up));

    m_rotateMatrix = glm::mat4(1.0f);

    m_dirty = true;
}

Camera::~Camera()
{
}

void Camera::update(Node& node)
{
    if (!m_enabled) return;

    const bool nodeChanged = m_nodeMatrixLevel != node.getMatrixLevel();
    if (!nodeChanged) return;

    m_worldPosition = node.getModelMatrix() * glm::vec4(m_position, 1.f);
    //m_viewPosition = node.getWorldPosition() + m_position;

    m_dirty = true;
    m_dirtyView = true;
    m_dirtyProjected = true;

    m_nodeMatrixLevel = node.getMatrixLevel();
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
    updateCamera();
    if (!m_dirtyProjected && !m_dirtyView) return m_projectedMatrix;
    m_projectedMatrix = m_projectionMatrix * getView();
    m_dirtyProjected = false;
    m_projectedLevel++;
    return m_projectedMatrix;
}

//
// NOTE KI - view right/up *CAN* be deduced from matrix
//
// - getViewRight() ==
//   glm::vec3 right{ view[0][0], view[1][0], view[2][0] };
//
// - getViewUp() ==
//   glm::vec3 up{ view[0][1], view[1][1], view[2][1] };
//
// - getPos() ==
//   glm::vec3 pos{ glm::inverse(ViewMatrix)[3] };
//
// @see https://gamedev.stackexchange.com/questions/5959/rendering-2d-sprites-into-a-3d-world
// This is the only interesting part of the tutorial.
// This is equivalent to mlutiplying (1,0,0) and (0,1,0) by inverse(ViewMatrix).
// ViewMatrix is orthogonal (it was made this way),
// so its inverse is also its transpose,
// and transposing a matrix is "free" (inversing is slooow)
//
const glm::mat4& Camera::getView() noexcept
{
    updateCamera();
    if (!m_dirtyView) return m_viewMatrix;

    m_viewMatrix = glm::lookAt(
        m_worldPosition,
        m_worldPosition + m_viewFront,
        m_viewUp);
    m_dirtyView = false;
    m_viewLevel++;

    return m_viewMatrix;
}

const glm::vec3& Camera::getViewFront() const noexcept
{
    if (m_dirty) updateCamera();
    return m_viewFront;
}

const glm::vec3& Camera::getViewRight() const noexcept
{
    if (m_dirty) updateCamera();
    return m_viewRight;
}

const glm::vec3& Camera::getViewUp() const noexcept
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

void Camera::setUp(const glm::vec3& up) noexcept
{
    if (m_up != up) {
        m_dirty = true;
        m_up = up;
    }
}

void Camera::setZoom(float zoom) noexcept
{
    updateZoom(zoom);
}

void Camera::adjustZoom(float adjustment) noexcept
{
    updateZoom(m_zoom - adjustment);
}

void Camera::setPosition(const glm::vec3& pos) noexcept
{
    if (m_position != pos) {
        m_position = pos;
        if (!m_nodeCamera) {
            m_worldPosition = pos;
        }
        m_dirty = true;
    }
}

void Camera::setRotation(const glm::vec3& rotation) noexcept
{
    if (m_rotation != rotation)
    {
        m_rotation = rotation;
        m_dirty = true;
    }
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

void Camera::updateCamera() const noexcept
{
    if (!m_dirty) return;
    m_dirty = false;

    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    m_rotateMatrix = glm::toMat4(glm::quat(glm::radians(m_rotation)));

    // NOTE KI glm::normalize for vec4 *IS* incorrect (4d len...)
    m_viewFront = glm::normalize(glm::vec3(m_rotateMatrix * glm::vec4(m_front, 1.f)));
    m_viewUp = glm::normalize(glm::vec3(m_rotateMatrix * glm::vec4(m_up, 1.f)));
    m_viewRight = glm::normalize(glm::cross(m_viewFront, m_viewUp));

    m_dirtyView = true;
    m_dirtyProjected = true;
}

