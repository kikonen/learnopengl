#include "Listen.h"

namespace event
{
    Listen::Listen()
    { }

    Listen::~Listen()
    {
        if (m_dispatcher) {
            m_dispatcher->removeListener(m_type, m_handle);
            m_dispatcher = nullptr;
        }
    }
}
