#include "UtilApi.h"

#include <fmt/format.h>

#include "ki/sid.h"
#include "ki/sid_format.h"
#include "util/Log.h"

#include "util/Log.h"
#include "util/glm_format.h"

namespace {
    struct CommandOptions {
        int index = 0;
        float duration = 0.f;
        float speed = 1.f;
        bool relative = false;
        bool repeat = false;
        bool sync = false;

        std::string name;

        std::string str() const noexcept {
            return fmt::format(
                "<OPT: duration={}, speed={}, relative={}, repeat={}, sync={}, name={}>",
                duration, speed, relative, repeat, sync, name);
        }
    };

    CommandOptions readOptions(const sol::table& lua_opt) noexcept {
        CommandOptions opt;
        lua_opt.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& k = key.as<std::string>();
            if (k == "index") {
                opt.index = value.as<int>();
            }
            else if (k == "time") {
                opt.duration = value.as<float>();
            }
            else if (k == "duration") {
                opt.duration = value.as<float>();
            }
            else if (k == "speed") {
                opt.speed = value.as<float>();
            }
            else if (k == "relative") {
                opt.relative = value.as<bool>();
            }
            else if (k == "loop") {
                opt.repeat = value.as<bool>();
            }
            else if (k == "sync") {
                opt.sync = value.as<bool>();
            }
            else if (k == "name") {
                opt.name = value.as<std::string>();
            }
            });
        return opt;
    }
}

namespace script
{
    UtilAPI::UtilAPI() = default;
    UtilAPI::~UtilAPI() = default;

    double UtilAPI::lua_sid(
        std::string id) noexcept
    {
        StringID sid{ id };
        return static_cast<double>(sid);
    }
}
