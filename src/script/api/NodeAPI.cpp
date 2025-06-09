#include "NodeAPI.h"

#include "model/Node.h"
#include "model/NodeState.h"
#include "model/NodeType.h"

#include "registry/NodeRegistry.h"

namespace script::api
{
    NodeAPI::NodeAPI(
        pool::NodeHandle handle)
        : m_handle{ handle },
        m_entityIndex{ handle.toNode()->m_entityIndex }
    {
    }

    NodeAPI::~NodeAPI() = default;

    std::string NodeAPI::str() const noexcept
    {
        return m_handle.toNode()->str();
    }

    ki::node_id NodeAPI::lua_get_id() const noexcept
    {
        return m_handle.m_id;
    }

    const std::string& NodeAPI::lua_get_type_name() const noexcept
    {
        return m_handle.toNode()->getType()->getName();
    }

    const std::string& NodeAPI::lua_get_name() const noexcept
    {
        return m_handle.toNode()->getName();
    }

    int NodeAPI::lua_get_clone_index() const noexcept
    {
        //return m_cloneIndex;
        return 0;
    }

    const glm::vec3& NodeAPI::lua_get_pos() const noexcept
    {
        return getState().getPosition();
    }

    const glm::vec3& NodeAPI::lua_get_front() const noexcept
    {
        return getState().getViewFront();
    }

    const glm::mat4& NodeAPI::lua_get_model_matrix() const noexcept
    {
        return getState().getModelMatrix();
    }

    const NodeState& NodeAPI::getState() const
    {
        return NodeRegistry::get().getState(m_entityIndex);
    }
}
