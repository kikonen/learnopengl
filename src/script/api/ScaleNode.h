#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

namespace script
{
    class ScaleNode final : public NodeCommand
    {
    public:
        ScaleNode(
            pool::NodeHandle handle,
            float duration,
            bool relative,
            const glm::vec3& scale) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "scale_node";
        }

        virtual void bind(
            const UpdateContext& ctx) noexcept override;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        glm::vec3 m_scale;
        glm::vec3 m_end{ 0.f };
        glm::vec3 m_previous{ 0.f };
    };
}
