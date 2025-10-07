#pragma once

#include "actions.h"

namespace event {
    struct Event {
        Event(Type a_type)
            : type{ a_type },
              body{}
        {}

        Event(Event& o) noexcept
            : type{ o.type },
            attachment{ std::move(o.attachment) },
            body{ o.body }
        {}

        Event(Event&& o) noexcept
            : type{ o.type },
            attachment{ std::move(o.attachment) },
            body{ o.body }
        {}

        ~Event();

        event::Attachment* attach()
        {
            if (!attachment) {
                attachment = std::make_unique<event::Attachment>();
            }
            return attachment.get();
        }

        std::unique_ptr<event::Attachment> attachment;

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
