#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "NodeCommand.h"

namespace script
{
    class MovePathNode final : public NodeCommand
    {
    public:
        MovePathNode(
            pool::NodeHandle handle,
            float duration,
            bool relative,
            const std::vector<glm::vec3>& path) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "move_path_node";
        }

        virtual void bind(
            const UpdateContext& ctx) noexcept override;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        void executeLeg(
            const UpdateContext& ctx,
            int leg);

    private:
        const std::vector<glm::vec3> m_path;

        int m_leg{ 0 };
        glm::vec3 m_end{ 0.f };
        glm::vec3 m_previous{ 0.f };
    };
}
