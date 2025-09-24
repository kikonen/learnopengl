#include "SplineCamera.h"

#include <glm/gtx/quaternion.hpp>

#include "util/debug.h"

#include "model/Node.h"

#include "render/Camera.h"

#include "render/RenderContext.h"

namespace {
    inline glm::vec3 UP{ 0, 1.f, 0 };
}

SplineCamera::SplineCamera() = default;
SplineCamera::~SplineCamera() = default;

void SplineCamera::updateRT(const UpdateContext& ctx, model::Node& node)
{
    if (!m_enabled) return;

    const auto dt = ctx.m_clock.elapsedSecs;

    if (!m_paused) {
        m_t += m_speed * dt;

        // Advance to the next control point if needed.
        // This assumes speed isn't so fast that you jump past
        // multiple control points in one frame.
        if (m_t >= 1.f)
        {
            // Make sure we have enough points to advance the path
            if (m_index < m_path.m_controlPoints.size() - 3)
            {
                m_index++;
                m_t -= 1.f;
            }
            else
            {
                // Path's done, so pause
                m_paused = true;
            }
        }
    }

    {
        // Camera position is the spline at the current t/index
        const auto& cameraPos = m_path.calculatePosition(m_index, m_t);

        // Target point is just a small delta ahead on the spline
        const auto& target = m_path.calculatePosition(m_index, m_t + 0.01f);

        // Assume spline doesn't flip upside-down
        const auto& cameraFront = glm::normalize(target - cameraPos);
        m_camera.setWorldPosition(cameraPos);
        m_camera.setAxis(cameraFront, UP);
    }
}

void SplineCamera::restart()
{
    m_index = 1;
    m_t = 0;
}
