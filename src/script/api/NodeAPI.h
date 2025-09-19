#pragma once

#include <string>

#include <glm/glm.hpp>

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "script/lua_binding.h"

struct NodeState;

namespace script::api
{
    class NodeAPI final
    {
    public:
        NodeAPI();
        ~NodeAPI();

        std::string str(pool::NodeHandle handle) const noexcept;

        pool::NodeHandle lua_find_child(
            pool::NodeHandle handle,
            const sol::table& lua_opt) const noexcept;

        const std::string& lua_get_type_name(
            pool::NodeHandle handle) const noexcept;

        const std::string& lua_get_name(
            pool::NodeHandle handle) const noexcept;

        int lua_get_clone_index(
            pool::NodeHandle handle) const noexcept;

        const glm::vec3& lua_get_pos(
            pool::NodeHandle handle) const noexcept;

        const glm::vec3& lua_get_front(
            pool::NodeHandle handle) const noexcept;

        const glm::mat4& lua_get_model_matrix(
            pool::NodeHandle handle) const noexcept;

        static void bind(sol::state& lua);

    private:
        const NodeState& getState(pool::NodeHandle handle) const;

    //private:
    //    const pool::NodeHandle m_handle;
    };
}
