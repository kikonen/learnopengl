#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/size.h"

#include "kigl/kigl.h"

#include "asset/Frustum.h"

namespace render {
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

        // NOtE KI setup via setViewport
        bool isOrthogonal() const noexcept
        {
            return m_orthogonal;
        }

        const std::array<float, 4>& getViewport() const noexcept {
            return m_viewport;
        }

        // NOTE KI set orthogonal viewport
        // left, right, bottom, top
        void setViewport(
            const std::array<float, 4>& viewport);

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
            return m_viewFront;
        }

        const glm::vec3& getViewRight() const noexcept
        {
            return m_viewRight;
        }

        const glm::vec3& getViewUp() const noexcept
        {
            return m_viewUp;
        }

        // @front Camera front (i.e. dir to target)
        // @param up *UP* direction of world
        void setAxis(
            const glm::vec3& front,
            const glm::vec3& up) noexcept;

        //inline const glm::vec3& getFront() const noexcept {
        //    return m_front;
        //}

        ////inline const glm::vec3& getRight() const noexcept {
        ////    return m_right;
        ////}

        //inline const glm::vec3& getUp() const noexcept {
        //    return m_up;
        //}

        inline float getFov() const noexcept {
            return m_fov;
        }

        void setFov(float fov) noexcept;
        void adjustFov(float adjustement) noexcept;

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

        void updateOrthogonalFrustum() const noexcept;
        void updatePerspectiveFrustum() const noexcept;

    private:
        // NOTE KI *ONLY* for orthogonal
        // left, right, bottom, top
        std::array<float, 4> m_viewport{ 0.f };

        float m_fov{ 45.f };
        float m_fovProjection = -1.0f;

        float m_aspectRatio = -1;
        float m_nearPlane = -1;
        float m_farPlane = -1;

        ki::level_id m_projectionLevel{ 0 };
        ki::level_id m_projectedLevel{ 0 };
        ki::level_id m_viewLevel{ 0 };

        bool m_orthogonal : 1 { false };

        glm::mat4 m_projectionMatrix{ 1.f };
        mutable glm::mat4 m_projectedMatrix{ 1.f };
        mutable glm::mat4 m_viewMatrix{ 1.f };

        glm::vec3 m_worldPosition{ 0.f };

        // *DIRECTION* at camera is pointing at (== target)
        // *NOT* required to be orthogonal to up
        glm::vec3 m_viewFront{ 0.f, 0.f, -1.f };

        // *UP* direction of camera
        glm::vec3 m_viewUp{ 0.f, 1.f, 0.f };

        glm::vec3 m_viewRight{ 0.f, 0.f, 1.f };

        mutable Frustum m_frustum;

        mutable bool m_dirty = true;
        mutable bool m_dirtyView = true;
        mutable bool m_dirtyProjected = true;
        mutable bool m_dirtyFrustum = true;
    };
}
