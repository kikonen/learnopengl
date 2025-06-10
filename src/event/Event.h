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
            blob{ std::move(o.blob) },
            body{ o.body }
        {}

        Event(Event&& o) noexcept
            : type{ o.type },
            blob{ std::move(o.blob) },
            body{ o.body }
        {}

        ~Event();

        Type type;

        std::unique_ptr<BlobData> blob;
        union Body {
            NodeAction node;
            NodeTypeAction nodeType;
            AudioSourceAction audioSource;
            AudioListenerAction audioListener;
            ScriptAction script;
            SelectAction select;
        } body;
    };

    struct EventPolicies
    {
        static Type getEvent(const Event& e) {
            return e.type;
        }
    };
}
