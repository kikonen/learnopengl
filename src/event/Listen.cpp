#include "Listen.h"

namespace event
{
    Listen::Listen(event::Type type)
        : m_type{ type }
    { }

    Listen::~Listen()
    {
        if (m_dispatcher) {
            m_dispatcher->removeListener(m_type, m_handle);
            m_dispatcher = nullptr;
        }
    }
}
