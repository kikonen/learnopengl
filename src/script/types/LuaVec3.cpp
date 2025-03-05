#include "LuaVec3.h"

namespace script
{
    void LuaVec3::bind(sol::state& state)
    {
        state.new_usertype<glm::vec3>("Vec3",
            sol::meta_function::construct,
            sol::factories(
                [](const double& x, const double& y, const double& z) {
                    return std::make_shared<glm::vec3>(x, y, z);
                }),
            "x",
            &glm::vec3::x,
            "y",
            &glm::vec3::y,
            "z",
            &glm::vec3::z);
    }
}
