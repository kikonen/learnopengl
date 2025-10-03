#pragma once

#include <functional>

#include "Type.h"
#include "Dispatcher.h"

namespace event
{
    class Listen
    {
    public:
        Listen(event::Type type);
        ~Listen();

        template <typename Callback>
        void listen(
            event::Dispatcher* dispatcher,
            Callback&& cb)
        {
            m_dispatcher = dispatcher;
            m_handle = dispatcher->addListener(m_type, std::forward<Callback>(cb));
        }

    private:
        const event::Type m_type;
        event::Dispatcher* m_dispatcher{ nullptr };
        event::Handle m_handle;
    };
}
