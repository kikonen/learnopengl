#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "NodeCommand.h"

namespace script
{
    class RotateNode final : public NodeCommand
    {
    public:
        RotateNode(
            pool::NodeHandle handle,
            float duration,
            bool relative,
            const glm::vec3& axis,
            const float degrees) noexcept;

        virtual void bind(
            const UpdateContext& ctx) noexcept override;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        glm::vec3 m_axis;
        float m_radians;
        float m_previousRadians{ 0.f };

        glm::quat m_base{ 1.f, 0.f, 0.f, 0.f };
        glm::vec3 m_relativeAxis{ 0.f };
    };
}
