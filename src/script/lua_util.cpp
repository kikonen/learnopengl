#include "lua_util.h"

#include <fmt/format.h>

namespace script {
    std::string CommandOptions::str() const noexcept
    {
        return fmt::format(
            "<OPT: after={}, duration={}, speed={}, relative={}, repeat={}, sync={}, name={}>",
            afterId, duration, speed, relative, repeat, sync, name);
    }

    std::string CommandEvent::str() const noexcept
    {
        return fmt::format(
            "<EVT: listenerId={}, type={}, data={}>",
            listenerId, type, data);
    }

    CommandOptions readOptions(const sol::table& lua_opt) noexcept
    {
        CommandOptions opt;
        lua_opt.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& k = key.as<std::string>();
            if (k == "after") {
                opt.afterId = value.as<script::command_id>();
            }
            else if (k == "sid") {
                opt.sid = value.as<unsigned int>();
            }
            else if (k == "node") {
                opt.nodeId = value.as<unsigned int>();
            }
            else if (k == "index") {
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
            else if (k == "count") {
                opt.count = value.as<float>();
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

    CommandEvent readEvent(const sol::table& lua_opt) noexcept
    {
        CommandEvent ev;
        lua_opt.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& k = key.as<std::string>();
            if (k == "listener") {
                ev.listenerId = value.as<int>();
            }
            else if (k == "type") {
                ev.type = value.as<int>();
            }
            else if (k == "data") {
                ev.data = value.as<std::string>();
            }
            });
        return ev;
    }

    std::vector<script::command_id> readCommandIds(const sol::table& v) noexcept
    {
        std::vector<script::command_id> ids;
        v.for_each([&](sol::object const& key, sol::object const& value) {
            int id = value.as<script::command_id>();
            ids.push_back(id);
            });
        return ids;
    }

    pool::NodeHandle getHandle(ki::node_id nodeId, pool::NodeHandle handle)
    {
        return nodeId > 0 ? pool::NodeHandle::toHandle(nodeId) : handle;
    }

    glm::vec3 readVec3(const sol::table& v) noexcept
    {
        return glm::vec3{ v.get<float>(1), v.get<float>(2), v.get<float>(3) };
    }
}
