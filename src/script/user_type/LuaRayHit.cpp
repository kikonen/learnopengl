#include "LuaRayHit.h"

#include <fmt/format.h>

#include "util/glm_format.h"

#include "physics/RayHit.h"

namespace script
{
    // https://thephd.dev/sol3-feature-complete
    void LuaRayHit::bind(sol::state& lua)
    {
        sol::usertype<physics::RayHit> t = lua.new_usertype<physics::RayHit>(
            "RayHit",
            "pos", &physics::RayHit::pos,
            "normal", &physics::RayHit::normal,
            "node_id", &physics::RayHit::handle,
            "is_hit", &physics::RayHit::isHit
            );

        t.set_function(
            "__tostring",
            [](const physics::RayHit& v) {
                return fmt::format("<node_id={}, pos={}, n={}, hit={}>", v.handle.toId(), v.pos, v.normal, v.isHit);
            }
        );
    }
}
