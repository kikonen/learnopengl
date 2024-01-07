#include "Dispatcher.h"

namespace event {
    Dispatcher::Dispatcher()
    {
    }

    void Dispatcher::prepare()
    {
    }

    void Dispatcher::dispatchEvents()
    {
        m_queue.process();
    }
}
