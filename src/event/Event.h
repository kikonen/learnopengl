#pragma once

#include "actions.h"

namespace event {
    struct Event {
        Event()
            : type{ Type::none },
            body{}
        {}

        Event(Type a_type)
            : type{ a_type },
              body{}
        {}

        Event(Event& o) noexcept
            : type{ o.type },
            attachment{ o.attachment },
            body{ o.body }
        {}

        Event(const Event& o) noexcept
            : type{ o.type },
            attachment{ o.attachment },
            body{ o.body }
        {}

        Event(Event&& o) noexcept
            : type{ o.type },
            attachment{ o.attachment },
            body{ o.body }
        {}

        ~Event();

        Event& operator=(const Event& o) noexcept
        {
            if (&o == this) return *this;

            type = o.type;
            attachment = o.attachment;
            body = o.body;
            return *this;
        }

        event::Attachment* attach()
        {
            if (!attachment) {
                attachment = std::make_shared<event::Attachment>();
            }
            return attachment.get();
        }

        std::shared_ptr<event::Attachment> attachment;

        union Body {
            NodeAction node;
            NodeTypeAction nodeType;
            ScriptAction script;
            SelectAction select;
            ViewportAction view;
        } body;

        Type type;
    };

    struct EventPolicies
    {
        static Type getEvent(const Event& e) {
            return e.type;
        }
    };
}
