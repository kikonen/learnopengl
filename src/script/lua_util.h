#pragma once

#include <string>

#include <glm/glm.hpp>

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "script/size.h"
#include "script/lua_binding.h"

namespace script
{
    struct CommandOptions {
        script::command_id afterId = 0;
        int index = 0;
        ki::sid_t sid = 0;
        //pool::NodeHandle nodeHandle{};
        ki::tag_id tagId = 0;
        float duration = 0.f;
        float speed = 1.f;
        float count = 0.f;
        bool relative = false;
        bool repeat = false;
        bool sync = false;
        bool self = false;

        std::string name;

        std::string str() const noexcept;
    };

    struct CommandEvent {
        int listenerId = 0;
        int type;
        std::string data;

        std::string str() const noexcept;
    };

    CommandOptions readOptions(const sol::table& lua_opt) noexcept;
    CommandEvent readEvent(const sol::table& lua_opt) noexcept;

    uint32_t readSID(const sol::object& v) noexcept;
    pool::NodeHandle readHandle(const sol::object& v) noexcept;

    std::vector<script::command_id> readCommandIds(const sol::table& v) noexcept;

    pool::NodeHandle selectHandle(
        pool::NodeHandle nodeHandle,
        pool::NodeHandle handle) noexcept;

    pool::NodeHandle selectHandle(
        pool::NodeHandle nodeHandle,
        pool::NodeHandle handle,
        ki::tag_id tagId) noexcept;

    pool::NodeHandle selectHandle(
        pool::NodeHandle handle,
        ki::tag_id tagId) noexcept;

    //glm::vec3 readVec3(const sol::table& v) noexcept;
}
