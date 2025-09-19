#include "UtilApi.h"

#include <fmt/format.h>

#include "ki/sid.h"
#include "ki/sid_format.h"
#include "util/Log.h"

#include "util/glm_util.h"
#include "util/glm_format.h"

#include "script/lua_util.h"

namespace {
}

namespace script::api
{
    //UtilAPI::UtilAPI() = default;
    //UtilAPI::~UtilAPI() = default;

    double UtilAPI::lua_sid(
        std::string id) noexcept
    {
        ki::StringID sid{ id };
        return static_cast<double>(sid);
    }

    const std::string& UtilAPI::lua_sid_name(
        ki::sid_t id)
    {
        return SID_NAME(id);
    }

    // https://thephd.dev/sol3-feature-complete
    void UtilAPI::bind(sol::state& lua)
    {
        auto t = lua.create_named_table("util");
        t["sid"] = &UtilAPI::lua_sid;
        t["sid_name"] = &UtilAPI::lua_sid_name;

        t.set_function(
            "lerp",
            sol::overload(
                [](float a, float b, float t) {
                    return std::lerp(a, b, t);
                },
                [](double a, double b, float t) {
                    return std::lerp(a, b, t);
                },
                [](int a, int b, float t) {
                    return std::lerp(a, b, t);
                }
            ));

        t.set_function(
            "clamp",
            sol::overload(
                [](float v, float min, float max) {
                    return std::clamp(v, min, max);
                },
                [](double v, double min, double max) {
                    return std::clamp(v, min, max);
                },
                [](int v, int min, int max) {
                    return std::clamp(v, min, max);
                }
            ));

        t.set_function(
            "degrees_between",
            sol::overload(
                [](const glm::vec3& v1, const glm::vec3& v2) {
                    return util::degreesBetween(v1, v2);
                }
            ));

        t.set_function(
            "radians_between",
            sol::overload(
                [](const glm::vec3& v1, const glm::vec3& v2) {
                    return util::radiansBetween(v1, v2);
                }
            ));

        t.set_function(
            "degrees_to_quat",
            sol::overload(
                [](const glm::vec3& rot) {
                    return util::degreesToQuat(rot);
                }
            ));

        t.set_function(
            "radians_to_quat",
            sol::overload(
                [](const glm::vec3& rot) {
                    return util::radiansToQuat(rot);
                }
            ));

        t.set_function(
            "quat_to_degrees",
            sol::overload(
                [](const glm::quat& rot) {
                    return util::quatToDegrees(rot);
                }
            ));

        t.set_function(
            "quat_to_radians",
            sol::overload(
                [](const glm::quat& rot) {
                    return util::quatToRadians(rot);
                }
            ));

        t.set_function(
            "radians_to_degrees",
            sol::overload(
                [](const glm::vec3& rot) {
                    return util::radiansToDegrees(rot);
                }
            ));

        t.set_function(
            "degrees_to_radians",
            sol::overload(
                [](const glm::vec3& rot) {
                    return util::degreesToRadians(rot);
                }
            ));

        t.set_function(
            "axis_degrees_to_quat",
            sol::overload(
                [](const glm::vec3& axis, float degrees) {
                    return util::axisDegreesToQuat(axis, degrees);
                }
            ));

        t.set_function(
            "axis_radians_to_quat",
            sol::overload(
                [](const glm::vec3& axis, float radians) {
                    return util::axisRadiansToQuat(axis, radians);
                }
            ));

        t.set_function(
            "normal_to_quat",
            sol::overload(
                [](const glm::vec3& normal, const glm::vec3& up) {
                    return util::normalToQuat(normal, up);
                }
            ));
    }
}
