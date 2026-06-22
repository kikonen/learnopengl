#pragma once

#include <functional>

#include "util/Ref.h"

#include "Type.h"
#include "Dispatcher.h"

namespace event
{
    class Listen
    {
    public:
        Listen();
        ~Listen();

        void listen(
            event::Type type,
            const util::Ref<event::Dispatcher>& dispatcher,
            Handler handler)
        {
            if (type == event::Type::none) return;
            if (m_type != event::Type::none) throw "EVENT::DUPLICATE_LISTENER";

            m_type = type;
            m_dispatcher = dispatcher;
            m_handle = dispatcher->addListener(m_type, std::move(handler));
        }

    private:
        util::Ref<event::Dispatcher> m_dispatcher{ nullptr };
        event::Handle m_handle;
        event::Type m_type{ event::Type::none };
    };
}
