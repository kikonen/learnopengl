#include "LuaGlm.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "glm/gtc/quaternion.hpp"

#include <fmt/format.h>

#include "util/glm_format.h"

namespace
{
    ///////////////////////////////////
    // VEC2
    void bindVec2(sol::state& lua)
    {
        auto multiply = sol::overload(
            [](const glm::vec2& v1, const glm::vec2& v2) { return v1 * v2; },
            [](const glm::vec2& v1, float value) { return v1 * value; },
            [](float value, const glm::vec2& v1) { return v1 * value; }
        );

        auto division = sol::overload(
            [](const glm::vec2& v1, const glm::vec2& v2) { return v1 / v2; },
            [](const glm::vec2& v1, float value) { return v1 / value; },
            [](float value, const glm::vec2& v1) { return v1 / value; }
        );

        auto addition = sol::overload(
            [](const glm::vec2& v1, const glm::vec2& v2) { return v1 + v2; },
            [](const glm::vec2& v1, float value) { return v1 + value; },
            [](float value, const glm::vec2& v1) { return v1 + value; }
        );

        auto subtraction = sol::overload(
            [](const glm::vec2& v1, const glm::vec2& v2) { return v1 - v2; },
            [](const glm::vec2& v1, float value) { return v1 - value; },
            [](float value, const glm::vec2& v1) { return v1 - value; }
        );

        auto t = lua.new_usertype<glm::vec2>(
            "vec2",
            sol::call_constructor,
            sol::constructors<
            glm::vec2(float),
            glm::vec2(float, float)>(),
            "x", &glm::vec2::x,
            "y", &glm::vec2::y,
            sol::meta_function::multiplication, multiply,
            sol::meta_function::division, division,
            sol::meta_function::addition, addition,
            sol::meta_function::subtraction, subtraction
        );

        t.set_function(
            "normalize",
            [](const glm::vec2& v) {
                return glm::normalize(v);
            }
        );

        t.set_function(
            "length",
            [](const glm::vec2& v) {
                return glm::length(v);
            }
        );

        t.set_function(
            "str",
            [](const glm::vec2& v) {
                return fmt::format("{}", v);
            }
        );

        t.set_function(
            "__tostring",
            [](const glm::vec2& v) {
                return fmt::format("{}", v);
            }
        );
    }

    ///////////////////////////////////
    // VEC3
    void bindVec3(sol::state& lua)
    {
        auto multiply = sol::overload(
            [](const glm::vec3& v1, const glm::vec3& v2) { return v1 * v2; },
            [](const glm::vec3& v1, float value) { return v1 * value; },
            [](float value, const glm::vec3& v1) { return v1 * value; }
        );

        auto division = sol::overload(
            [](const glm::vec3& v1, const glm::vec3& v2) { return v1 / v2; },
            [](const glm::vec3& v1, float value) { return v1 / value; },
            [](float value, const glm::vec3& v1) { return v1 / value; }
        );

        auto addition = sol::overload(
            [](const glm::vec3& v1, const glm::vec3& v2) { return v1 + v2; },
            [](const glm::vec3& v1, float value) { return v1 + value; },
            [](float value, const glm::vec3& v1) { return v1 + value; }
        );

        auto subtraction = sol::overload(
            [](const glm::vec3& v1, const glm::vec3& v2) { return v1 - v2; },
            [](const glm::vec3& v1, float value) { return v1 - value; },
            [](float value, const glm::vec3& v1) { return v1 - value; }
        );

        auto t = lua.new_usertype<glm::vec3>(
            "vec3",
            sol::call_constructor,
            sol::constructors<
            glm::vec3(float),
            glm::vec3(float, float, float),
            glm::vec3(const glm::vec4&)>(),
            "x", &glm::vec3::x,
            "y", &glm::vec3::y,
            "z", &glm::vec3::z,
            sol::meta_function::multiplication, multiply,
            sol::meta_function::division, division,
            sol::meta_function::addition, addition,
            sol::meta_function::subtraction, subtraction
        );

        t.set_function(
            "normalize",
            [](const glm::vec3& v) {
                return glm::normalize(v);
            }
        );

        t.set_function(
            "length",
            [](const glm::vec3& v) {
                return glm::length(v);
            }
        );

        t.set_function(
            "str",
            [](const glm::vec3& v) {
                return fmt::format("{}", v);
            }
        );

        t.set_function(
            "__tostring",
            [](const glm::vec3& v) {
                return fmt::format("{}", v);
            }
        );
    }

    ///////////////////////////////////
    // VEC4
    void bindVec4(sol::state& lua)
    {
        auto multiply = sol::overload(
            [](const glm::vec4& v1, const glm::vec4& v2) { return v1 * v2; },
            [](const glm::vec4& v1, float value) { return v1 * value; },
            [](float value, const glm::vec4& v1) { return v1 * value; }
        );

        auto division = sol::overload(
            [](const glm::vec4& v1, const glm::vec4& v2) { return v1 / v2; },
            [](const glm::vec4& v1, float value) { return v1 / value; },
            [](float value, const glm::vec4& v1) { return v1 / value; }
        );

        auto addition = sol::overload(
            [](const glm::vec4& v1, const glm::vec4& v2) { return v1 + v2; },
            [](const glm::vec4& v1, float value) { return v1 + value; },
            [](float value, const glm::vec4& v1) { return v1 + value; }
        );

        auto subtraction = sol::overload(
            [](const glm::vec4& v1, const glm::vec4& v2) { return v1 - v2; },
            [](const glm::vec4& v1, float value) { return v1 - value; },
            [](float value, const glm::vec4& v1) { return v1 - value; }
        );

        auto t = lua.new_usertype<glm::vec4>(
            "vec4",
            sol::call_constructor,
            sol::constructors<
            glm::vec4(float),
            glm::vec4(float, float, float, float),
            glm::vec4(const glm::vec3&, float)>(),
            "x", &glm::vec4::x,
            "y", &glm::vec4::y,
            "z", &glm::vec4::z,
            "w", &glm::vec4::w,
            sol::meta_function::multiplication, multiply,
            sol::meta_function::division, division,
            sol::meta_function::addition, addition,
            sol::meta_function::subtraction, subtraction
        );

        t.set_function(
            "normalize",
            [](const glm::vec4& v) {
                return glm::normalize(v);
            }
        );

        t.set_function(
            "length",
            [](const glm::vec4& v) {
                return glm::length(v);
            }
        );

        t.set_function(
            "str",
            [](const glm::vec4& v) {
                return fmt::format("{}", v);
            }
        );

        t.set_function(
            "__tostring",
            [](const glm::vec4& v) {
                return fmt::format("{}", v);
            }
        );
    }

    ///////////////////////////////////
    // MAT3
    void bindMat3(sol::state& lua)
    {
        auto multiply = sol::overload(
            [](const glm::mat3& m1, const glm::mat3& m2) { return m1 * m2; },
            [](const glm::mat3& m1, const glm::vec3& v2) { return m1 * v2; },
            [](const glm::mat3& m1, float value) { return m1 * value; },
            [](float value, const glm::mat3& m1) { return m1 * value; }
        );

        auto division = sol::overload(
            [](const glm::mat3& m1, const glm::mat3& m2) { return m1 / m2; },
            [](const glm::mat3& m1, float value) { return m1 / value; },
            [](float value, const glm::mat3& m1) { return m1 / value; }
        );

        auto addition = sol::overload(
            [](const glm::mat3& m1, const glm::mat3& m2) { return m1 + m2; },
            [](const glm::mat3& m1, float value) { return m1 + value; },
            [](float value, const glm::mat3& m1) { return m1 + value; }
        );

        auto subtraction = sol::overload(
            [](const glm::mat3& m1, const glm::mat3& m2) { return m1 - m2; },
            [](const glm::mat3& m1, float value) { return m1 - value; },
            [](float value, const glm::mat3& m1) { return m1 - value; }
        );

        auto t = lua.new_usertype<glm::mat3>(
            "mat3",
            sol::call_constructor,
            sol::constructors<
            glm::mat3(float),
            glm::mat3(const glm::mat4&)>(),
            sol::meta_function::multiplication, multiply,
            sol::meta_function::division, division,
            sol::meta_function::addition, addition,
            sol::meta_function::subtraction, subtraction
        );

        t.set_function(
            "inverse",
            [](const glm::mat3& v) {
                return glm::inverse(v);
            }
        );

        t.set_function(
            "transpose",
            [](const glm::mat3& v) {
                return glm::transpose(v);
            }
        );

        t.set_function(
            "inverse_transpose",
            [](const glm::mat3& v) {
                return glm::inverseTranspose(v);
            }
        );

        t.set_function(
            "str",
            [](const glm::mat3& v) {
                return fmt::format("{}", v);
            }
        );

        t.set_function(
            "__tostring",
            [](const glm::mat3& v) {
                return fmt::format("{}", v);
            }
        );
    }

    ///////////////////////////////////
    // MAT4
    void bindMat4(sol::state& lua)
    {
        auto multiply = sol::overload(
            [](const glm::mat4& m1, const glm::mat4& m2) { return m1 * m2; },
            [](const glm::mat4& m1, const glm::vec4& v2) { return m1 * v2; },
            [](const glm::mat4& m1, float value) { return m1 * value; },
            [](float value, const glm::mat4& m1) { return m1 * value; }
        );

        auto division = sol::overload(
            [](const glm::mat4& m1, const glm::mat4& m2) { return m1 / m2; },
            [](const glm::mat4& m1, float value) { return m1 / value; },
            [](float value, const glm::mat4& m1) { return m1 / value; }
        );

        auto addition = sol::overload(
            [](const glm::mat4& m1, const glm::mat4& m2) { return m1 + m2; },
            [](const glm::mat4& m1, float value) { return m1 + value; },
            [](float value, const glm::mat4& m1) { return m1 + value; }
        );

        auto subtraction = sol::overload(
            [](const glm::mat4& m1, const glm::mat4& m2) { return m1 - m2; },
            [](const glm::mat4& m1, float value) { return m1 - value; },
            [](float value, const glm::mat4& m1) { return m1 - value; }
        );

        auto t = lua.new_usertype<glm::mat4>(
            "mat4",
            sol::call_constructor,
            sol::constructors<
            glm::mat4(float),
            glm::mat4(const glm::mat3&)>(),
            sol::meta_function::multiplication, multiply,
            sol::meta_function::division, division,
            sol::meta_function::addition, addition,
            sol::meta_function::subtraction, subtraction
        );

        t.set_function(
            "col",
            [](const glm::mat4& v, int column) {
                return fmt::format("{}", v);
            }
        );

        t.set_function(
            "inverse",
            [](const glm::mat4& v) {
                return glm::inverse(v);
            }
        );

        t.set_function(
            "transpose",
            [](const glm::mat4& v) {
                return glm::transpose(v);
            }
        );

        t.set_function(
            "inverse_transpose",
            [](const glm::mat4& v) {
                return glm::inverseTranspose(v);
            }
        );

        t.set_function(
            "str",
            [](const glm::mat4& v) {
                return fmt::format("{}", v);
            }
        );

        t.set_function(
            "__tostring",
            [](const glm::mat4& v) {
                return fmt::format("{}", v);
            }
        );
    }

    ///////////////////////////////////
    // API
    void bindGlm(sol::state& lua) {
        auto t = lua.create_named_table("glm");

        //t.set_function(
        //    "normalize",
        //    sol::overload(
        //        [](const glm::vec3& v) {
        //            return glm::normalize(v);
        //        }
        //    ));

        //t.set_function(
        //    "length",
        //    sol::overload(
        //        [](const glm::vec2& v) {
        //            return glm::length(v);
        //        },
        //        [](const glm::vec3& v) {
        //            return glm::length(v);
        //        }
        //    ));

        t.set_function(
            "dot",
            sol::overload(
                [](const glm::vec2& v1, const glm::vec2& v2) {
                    return glm::dot(v1, v2);
                },
                [](const glm::vec3& v1, const glm::vec3& v2) {
                    return glm::dot(v1, v2);
                }
            ));

        t.set_function(
            "cross",
            sol::overload(
                [](const glm::vec3& v1, const glm::vec3& v2) {
                    return glm::cross(v1, v2);
                }
            ));

        t.set_function(
            "distance",
            sol::overload(
                [](const glm::vec2& v1, const glm::vec2& v2) {
                    return glm::distance(v1, v2);
                },
                [](const glm::vec3& v1, const glm::vec3& v2) {
                    return glm::distance(v1, v2);
                }
            ));

        t.set_function(
            "distance2",
            sol::overload(
                [](const glm::vec2& v1, const glm::vec2& v2) {
                    return glm::distance2(v1, v2);
                },
                [](const glm::vec3& v1, const glm::vec3& v2) {
                    return glm::distance2(v1, v2);
                }
            ));
    }
}

namespace script
{
    void LuaGlm::bind(sol::state& lua)
    {
        bindVec2(lua);
        bindVec3(lua);
        bindVec4(lua);

        bindMat3(lua);
        bindMat4(lua);

        bindGlm(lua);
    }
}
