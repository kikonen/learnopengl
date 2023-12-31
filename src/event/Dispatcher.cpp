#include "Dispatcher.h"

namespace event {
    Dispatcher::Dispatcher(const Assets& assets)
        : m_assets(assets)
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
