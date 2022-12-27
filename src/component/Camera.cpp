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
    m_dirtyFrustum = true;
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

const Frustum& Camera::getFrustum() noexcept
{
    if (m_dirtyFrustum) updateFrustum();
    return m_frustum;
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

void Camera::adjustZoom(float adjustment) noexcept
{
    updateZoom(m_zoom - adjustment);
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
    m_dirtyFrustum = true;
    m_viewLevel++;
}

void Camera::updateFrustum() noexcept
{
    // TODO KI https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    // https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/entity.h

    // NOTE KI use 90 angle for culling; smaller does cut-off too early
    // => 90 angle neither working correctly always for terrain tiles
    // => TODO KI WHAT is failing
    const float fovY = glm::radians(getZoom());
    //const float fovY = glm::radians(90.f);
    const glm::vec3& pos = getPos();
    const glm::vec3& front = getViewFront();
    const glm::vec3& up = getViewUp();
    const glm::vec3& right = getViewRight();

    const float halfVSide = m_farPlane * tanf(fovY * .5f);
    const float halfHSide = halfVSide * m_aspectRatio;
    const glm::vec3 frontMultFar = m_farPlane * front;

    // NOTE KI near and far plane don't have camee pos as "point in plane"
    // NOTE KI other side HAVE camra pos as "point in plane"

    m_frustum.nearFace = {
        pos + m_nearPlane * front,
        front };

    m_frustum.farFace = {
        pos + frontMultFar,
        -front };

    m_frustum.rightFace = {
        pos,
        glm::cross(up, frontMultFar + right * halfHSide) };

    m_frustum.leftFace = {
        pos,
        glm::cross(frontMultFar - right * halfHSide, up) };

    m_frustum.topFace = {
        pos,
        glm::cross(right, frontMultFar - up * halfVSide) };

    m_frustum.bottomFace = {
        pos,
        glm::cross(frontMultFar + up * halfVSide, right) };

    m_dirtyFrustum = false;
}
