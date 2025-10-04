#pragma once

#include <functional>

#include "Type.h"
#include "Dispatcher.h"

namespace event
{
    class Listen
    {
    public:
        Listen();
        ~Listen();

        template <typename Callback>
        void listen(
            event::Type type,
            event::Dispatcher* dispatcher,
            Callback&& cb)
        {
            if (type == event::Type::none) return;
            if (m_type != event::Type::none) throw "duplicate event register";

            m_type = type;
            m_dispatcher = dispatcher;
            m_handle = dispatcher->addListener(m_type, std::forward<Callback>(cb));
        }

    private:
        event::Type m_type{ event::Type::none };
        event::Dispatcher* m_dispatcher{ nullptr };
        event::Handle m_handle;
    };
}
