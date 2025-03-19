#include "LuaRayHit.h"

#include <fmt/format.h>

#include "util/glm_format.h"

#include "physics/RayHit.h"

namespace script
{
    // https://thephd.dev/sol3-feature-complete
    void LuaRayHit::bind(sol::state& lua)
    {
        sol::usertype<physics::RayHit> t = lua.new_usertype<physics::RayHit>("RayHit");

        t.set_function(
            "__tostring",
            [](const physics::RayHit& v) {
                return fmt::format("<node_id={}, pos={}, n={}>", v.handle.toId(), v.pos, v.normal);
            }
        );
    }
}
