#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

namespace script
{
    class MoveNode final : public NodeCommand
    {
    public:
        //MoveNode()
        //    : NodeCommand(0, 0, true),
        //    m_position(0)
        //{}

        MoveNode(
            pool::NodeHandle handle,
            float duration,
            bool relative,
            const glm::vec3& position) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "move_node";
        }

        virtual void bind(
            const UpdateContext& ctx) noexcept override;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        glm::vec3 m_position;

        glm::vec3 m_end{ 0.f };
        glm::vec3 m_previous{ 0.f };
    };
}
