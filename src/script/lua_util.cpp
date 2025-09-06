#include "lua_util.h"

#include <fmt/format.h>

#include "registry/NodeRegistry.h"

namespace {
    const inline std::string OPT_AFTER{ "after" };
    const inline std::string OPT_TIME{ "time" };
    const inline std::string OPT_RELATIVE{ "relative" };
    const inline std::string OPT_SID{ "sid" };
    const inline std::string OPT_TAG{ "tag" };
    const inline std::string OPT_INDEX{ "index" };
    const inline std::string OPT_DURATION{ "duration" };
    const inline std::string OPT_SPEED{ "speed" };
    const inline std::string OPT_COUNT{ "count" };
    const inline std::string OPT_LOOP{ "loop" };
    const inline std::string OPT_SYNC{ "sync" };
    const inline std::string OPT_OBJECT{ "object" };
    const inline std::string OPT_NAME{ "name" };
    const inline std::string OPT_NODE{ "node" };

    const inline std::string OPT_LISTENER{ "listener" };
    const inline std::string OPT_TYPE{ "type" };
    const inline std::string OPT_DATA{ "data" };
}

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
            if (k == OPT_AFTER) {
                opt.afterId = value.as<script::command_id>();
            }
            else if (k == OPT_TIME) {
                opt.duration = value.as<float>();
            }
            else if (k == OPT_RELATIVE) {
                opt.relative = value.as<bool>();
            }
            else if (k == OPT_SID) {
                opt.sid = value.as<unsigned int>();
            }
            else if (k == OPT_TAG) {
                opt.tagId = value.as<unsigned int>();
            }
            else if (k == OPT_INDEX) {
                opt.index = value.as<int>();
            }
            else if (k == OPT_DURATION) {
                opt.duration = value.as<float>();
            }
            else if (k == OPT_SPEED) {
                opt.speed = value.as<float>();
            }
            else if (k == OPT_COUNT) {
                opt.count = value.as<float>();
            }
            else if (k == OPT_LOOP) {
                opt.repeat = value.as<bool>();
            }
            else if (k == OPT_SYNC) {
                opt.sync = value.as<bool>();
            }
            else if (k == OPT_OBJECT) {
                opt.self = value.as<bool>();
            }
            else if (k == OPT_NAME) {
                opt.name = value.as<std::string>();
            }
            else if (k == OPT_NODE) {
                opt.nodeId = value.as<unsigned int>();
            }
            });
        return opt;
    }

    CommandEvent readEvent(const sol::table& lua_opt) noexcept
    {
        CommandEvent ev;
        lua_opt.for_each([&](sol::object const& key, sol::object const& value) {
            const auto& k = key.as<std::string>();
            if (k == OPT_LISTENER) {
                ev.listenerId = value.as<int>();
            }
            else if (k == OPT_TYPE) {
                ev.type = value.as<int>();
            }
            else if (k == OPT_DATA) {
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

    pool::NodeHandle getHandle(
        ki::node_id nodeId,
        pool::NodeHandle handle) noexcept
    {
        return nodeId > 0 ? pool::NodeHandle::toHandle(nodeId) : handle;
    }

    pool::NodeHandle getHandle(
        ki::node_id nodeId,
        pool::NodeHandle handle,
        ki::tag_id tagId) noexcept
    {
        if (nodeId > 0) {
            handle = pool::NodeHandle::toHandle(nodeId);
        }
        if (tagId > 0) {
            handle = NodeRegistry::get().findTaggedChild(handle, tagId);
        }
        return handle;
    }

    //glm::vec3 readVec3(const sol::table& v) noexcept
    //{
    //    return glm::vec3{ v.get<float>(1), v.get<float>(2), v.get<float>(3) };
    //}
}
