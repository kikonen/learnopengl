#include "Camera.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glm/ext.hpp>

#include "model/Node.h"
#include "render/RenderContext.h"

const float MIN_FOV = 10.0f;
// NOTE KI 90 to allow cubemap & shadowmap wide angle
const float MAX_FOV = 90.0f;


Camera::Camera()
{}

Camera::Camera(
    const glm::vec3& worldPos,
    const glm::vec3 front,
    const glm::vec3 up)
{
    m_enabled = true;
    m_position = worldPos;
    m_worldPosition = worldPos;

    setAxis(front, up);
}

void Camera::update(const UpdateContext& ctx, Node& node) noexcept
{
    if (!m_enabled) return;

    const bool nodeChanged = m_nodeLevel != node.getMatrixLevel();
    if (!nodeChanged) return;

    m_nodeModelMatrix = node.getModelMatrix();

    // NOTE KI *IGNORE* scale
    const auto& scale = node.getScale();
    m_nodeModelMatrix[0][0] /= scale[0];
    m_nodeModelMatrix[1][1] /= scale[1];
    m_nodeModelMatrix[2][2] /= scale[2];

    m_worldPosition = m_nodeModelMatrix * glm::vec4(m_position, 1.f);

    m_dirty = true;
    m_dirtyView = true;
    m_dirtyProjected = true;

    m_nodeLevel = node.getMatrixLevel();
}

void Camera::setViewport(
    const std::array<float, 4>& viewport)
{
    m_viewport = viewport;
    m_orthagonal = true;
    m_dirty = true;
}

void Camera::setupProjection(
    float aspectRatio,
    float nearPlane,
    float farPlane)
{
    if (m_aspectRatio == aspectRatio &&
        m_nearPlane == nearPlane &&
        m_farPlane == farPlane &&
        m_fovProjection == m_fov) return;

    m_aspectRatio = aspectRatio;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_fovProjection = m_fov;

    if (m_orthagonal) {
        m_projectionMatrix = glm::ortho(
            m_viewport[0], m_viewport[1], m_viewport[2], m_viewport[3],
            m_nearPlane,
            m_farPlane);
    } else {
        m_projectionMatrix = glm::perspective(
            glm::radians(m_fovProjection),
            m_aspectRatio,
            m_nearPlane,
            m_farPlane);
    }

    m_projectionLevel++;
    m_dirtyProjected = true;
}

const glm::mat4& Camera::getProjection() const noexcept
{
    return m_projectionMatrix;
}

const glm::mat4& Camera::getProjected() noexcept
{
    if (m_dirty) updateCamera();
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
    if (m_dirty) updateCamera();
    if (!m_dirtyView) return m_viewMatrix;

    m_viewMatrix = glm::lookAt(
        m_worldPosition,
        m_worldPosition + m_viewFront,
        m_viewUp);
    m_dirtyView = false;
    m_viewLevel++;

    return m_viewMatrix;
}

void Camera::setAxis(
    const glm::vec3& front,
    const glm::vec3& up) noexcept
{
    m_front = glm::normalize(front);
    m_up = glm::normalize(up);
    //m_right = glm::normalize(glm::cross(m_front, m_up));
    m_dirty = true;
}

void Camera::setFov(float fov) noexcept
{
    updateFov(fov);
}

void Camera::adjustFov(float adjustment) noexcept
{
    updateFov(m_fov - adjustment);
}

void Camera::setWorldPosition(const glm::vec3& pos) noexcept
{
    if (m_worldPosition != pos) {
        m_worldPosition = pos;
        m_dirty = true;
    }
}

void Camera::setPosition(const glm::vec3& pos) noexcept
{
    if (m_position != pos) {
        m_position = pos;
        m_worldPosition = pos;
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

void Camera::updateFov(float fov) noexcept
{
    if (fov < MIN_FOV) {
        fov = MIN_FOV;
    }
    if (fov > MAX_FOV) {
        fov = MAX_FOV;
    }
    if (m_fov != fov) {
        m_fov = fov;
        m_dirty = true;
    }
}

void Camera::updateCamera() const noexcept
{
    if (!m_dirty) return;

    glm::vec3 viewFront = m_front;
    //if (m_rotation.x != 0 || m_rotation.y != 0 || m_rotation.z != 0)
    {
        // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
        m_rotateMatrix = glm::toMat4(glm::quat(glm::radians(m_rotation)));

        glm::mat4 modelMatrix = m_nodeModelMatrix * m_rotateMatrix;
        // NOTE KI remove translate
        modelMatrix[3] = { 0.f, 0.f, 0.f, 1.f };

        viewFront = glm::vec3(modelMatrix * glm::vec4(m_front, 1.f));
    }

    // NOTE KI glm::normalize for vec4 *IS* incorrect (4d len...)
    m_viewFront = glm::normalize(viewFront);
    m_viewRight = glm::normalize(glm::cross(m_viewFront, m_up));
    m_viewUp = glm::normalize(glm::cross(m_viewRight, m_viewFront));

    m_dirty = false;
    m_dirtyView = true;
    m_dirtyProjected = true;
    m_dirtyFrustum = true;
}

/* https://ogldev.org/www/tutorial13/tutorial13.html
void Camera::updateUVN()
{
    Vector3f N = Target;
    N.Normalize();
    Vector3f U = Up;
    U = U.Cross(Target);
    U.Normalize();
    Vector3f V = N.Cross(U);

    m[0][0] = U.x; m[0][1] = U.y; m[0][2] = U.z; m[0][3] = 0.0f;
    m[1][0] = V.x; m[1][1] = V.y; m[1][2] = V.z; m[1][3] = 0.0f;
    m[2][0] = N.x; m[2][1] = N.y; m[2][2] = N.z; m[2][3] = 0.0f;
    m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
}
*/

void Camera::updateFrustum() const noexcept
{
    if (m_orthagonal) {
        updateOrthagonalFrustum();
    }
    else {
        updatePerspectiveFrustum();
    }
}

void Camera::updateOrthagonalFrustum() const noexcept
{
    const auto& viewport = getViewport();
    const auto& pos = getWorldPosition();
    const auto& front = getViewFront();
    const auto& up = getViewUp();
    const auto& right = getViewRight();

    m_frustum.nearFace = {
        pos + m_nearPlane * front,
        front };

    m_frustum.farFace = {
        pos + m_farPlane * front,
        -front };

    m_frustum.leftFace = {
        pos + viewport[0] * right,
        right };

    m_frustum.rightFace = {
        pos + viewport[1] * right,
        -right };

    m_frustum.bottomFace = {
        pos + viewport[2] * up,
        up };

    m_frustum.topFace = {
        pos + viewport[3] * up,
        -up };

    m_dirtyFrustum = false;
}

void Camera::updatePerspectiveFrustum() const noexcept
{
    // TODO KI https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    // https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/entity.h

    const glm::vec3& pos = getWorldPosition();
    const glm::vec3& front = getViewFront();
    const glm::vec3& up = getViewUp();
    const glm::vec3& right = getViewRight();

    const float halfVSide = m_farPlane * tanf(glm::radians(getFov()) * .5f);
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

    m_frustum.leftFace = {
        pos,
        glm::cross(frontMultFar - right * halfHSide, up) };

    m_frustum.rightFace = {
        pos,
        glm::cross(up, frontMultFar + right * halfHSide) };

    m_frustum.topFace = {
        pos,
        glm::cross(right, frontMultFar - up * halfVSide) };

    m_frustum.bottomFace = {
        pos,
        glm::cross(frontMultFar + up * halfVSide, right) };

    m_dirtyFrustum = false;
}
