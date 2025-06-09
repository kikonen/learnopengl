#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

namespace script
{
    class MoveSplineNode final : public NodeCommand
    {
    public:
        MoveSplineNode(
            pool::NodeHandle handle,
            float duration,
            bool relative,
            const glm::vec3& controlPoint,
            const glm::vec3& position) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "move_spline_node";
        }

        virtual void bind(
            const UpdateContext& ctx) noexcept override;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        glm::vec3 m_controlPoint;
        glm::vec3 m_position;

        glm::vec3 m_end{ 0.f };
        glm::vec3 m_previous{ 0.f };
    };
}
