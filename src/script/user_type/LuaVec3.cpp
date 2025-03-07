#include "LuaVec3.h"

namespace script
{
    void LuaVec3::bind(sol::state& state)
    {
        state.new_usertype<glm::vec3>("Vec3",
            sol::meta_function::construct,
            sol::factories(
                [](const float& x, const float& y, const float& z) {
                    return glm::vec3{ x, y, z };
                }),
            "x",
            &glm::vec3::x,
            "y",
            &glm::vec3::y,
            "z",
            &glm::vec3::z,
            "len",
            &glm::vec3::length,
            "normalize",
            [](const glm::vec3& v) {
                return glm::normalize(v);
            }
            );
    }
}
