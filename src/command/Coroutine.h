#pragma once

#include <memory>
#include <sol/sol.hpp>

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
