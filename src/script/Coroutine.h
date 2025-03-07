#pragma once

#include <memory>

#include "script/lua_binding.h"

namespace script
{
    // @see references
    // - https://sol2.readthedocs.io/en/latest/api/coroutine.html
    // - https://github.com/ThePhD/sol2/issues/528
    class Coroutine {
    public:
        Coroutine(
            sol::state& lua,
            sol::function& fn,
            size_t id);

        ~Coroutine();

        const size_t m_id;
        sol::thread m_thread;
        sol::coroutine* m_coroutine;
    };
}
