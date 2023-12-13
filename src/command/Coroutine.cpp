#include "Coroutine.h"

Coroutine::Coroutine(
    sol::state& lua,
    sol::function& fn,
    size_t id)
    : m_id(id),
    m_thread{ sol::thread::create(lua) },
    m_coroutine{ new sol::coroutine(m_thread.state(), fn) }
{
}



Coroutine::~Coroutine()
{
    delete m_coroutine;
}
