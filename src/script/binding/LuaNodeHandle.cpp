#include "LuaNodeHandle.h"

#include "pool/NodeHandle.h"

namespace script::binding
{
    void LuaNodeHandle::bind(sol::state& lua)
    {
        sol::usertype<pool::NodeHandle> t = lua.new_usertype<pool::NodeHandle>(
            "NodeHandle",
            "id", &pool::NodeHandle::m_id,
            "index", &pool::NodeHandle::m_handleIndex
        );

        t.set_function(
            "__tostring",
            [](const pool::NodeHandle& v) {
                return v.str();
            }
        );
    }
}
