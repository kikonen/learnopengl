#pragma once

#include <functional>

#include <glm/glm.hpp>

#include "NodeCommand.h"

namespace nav
{
    struct Path;
}

namespace script
{
    class FindPath : public NodeCommand
    {
    public:
        FindPath(
            pool::NodeHandle handle,
            const glm::vec3& startPos,
            const glm::vec3& endPos,
            int maPath,
            const std::function<void(int cid, const nav::Path&)>& callback) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "find_path";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        const glm::vec3 m_startPos;
        const glm::vec3 m_endPos;
        const int m_maxPath;

        const std::function<void(int cid, const nav::Path&)> m_callback;
    };
}
