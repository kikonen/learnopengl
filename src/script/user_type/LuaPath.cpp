#include "LuaPath.h"

#include <fmt/format.h>

#include "util/glm_format.h"

#include "nav/Path.h"

namespace script
{
    // https://thephd.dev/sol3-feature-complete
    void LuaPath::bind(sol::state& lua)
    {
        sol::usertype<nav::Path> t = lua.new_usertype<nav::Path>(
            "Path",
            "start_pos", &nav::Path::m_startPos,
            "end_pos ", &nav::Path::m_endPos,
            "waypoints", &nav::Path::m_waypoints
        );

        t.set_function(
            "len",
            [](const nav::Path& v) {
                return v.m_waypoints.size();
            }
        );

        t.set_function(
            "waypoint",
            [](const nav::Path& v, int index) {
                return v.m_waypoints[index];
            }
        );

        t.set_function(
            "__tostring",
            [](const nav::Path& v) {
                return fmt::format(
                    "<start={}, end={}, len={}>",
                    v.m_startPos, v.m_endPos, v.m_waypoints.size());
            }
        );
    }
}
