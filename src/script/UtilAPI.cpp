#include "UtilApi.h"

#include <fmt/format.h>

#include "ki/sid.h"
#include "ki/sid_format.h"
#include "util/Log.h"

#include "util/Log.h"
#include "util/glm_format.h"

#include "lua_util.h"

namespace {
}

namespace script
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
}
