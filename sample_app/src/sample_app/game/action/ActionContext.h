#pragma once

#include <glm/glm.hpp>

#include "pool/NodeHandle.h"

#include "engine/BaseContext.h"

namespace action
{
    struct ActionContext final : BaseContext
    {
    public:
        ActionContext(
            Engine& engine,
            const pool::NodeHandle& handle,
            const glm::vec3& pos,
            const glm::vec3& dir);

    public:
        const pool::NodeHandle m_handle;
        const glm::vec3 m_pos;
        const glm::vec3 m_dir;
    };
}
