#pragma once

#include <string>

#include <glm/glm.hpp>

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "script/lua_binding.h"

struct NodeState;

namespace script
{
    class NodeAPI final
    {
    public:
        NodeAPI(
            pool::NodeHandle handle);
        ~NodeAPI();

        std::string str() const noexcept;

        ki::node_id lua_get_id() const noexcept;

        const std::string& lua_get_type_name() const noexcept;
        const std::string& lua_get_name() const noexcept;

        int lua_get_clone_index() const noexcept;

        const glm::vec3& lua_get_pos() const noexcept;

        const glm::vec3& lua_get_front() const noexcept;

        const glm::mat4& lua_get_model_matrix() const noexcept;

    private:
        const NodeState& getState() const;

    private:
        const pool::NodeHandle m_handle;
        const uint32_t m_entityIndex;
    };
}
