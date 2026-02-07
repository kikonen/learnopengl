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

        Event(Event& o) = delete;
        Event(const Event& o) = delete;

        Event(Event&& o) noexcept
            : type{ o.type },
            attachment{ std::move(o.attachment) },
            body{ o.body }
        {
            o.attachment = nullptr;
        }

        ~Event();

        Event& operator=(const Event& o) noexcept = delete;
        Event& operator=( Event& o) noexcept = delete;

        Event& operator=(Event&& o) noexcept
        {
            if (&o == this) return *this;

            type = o.type;
            attachment = std::move(o.attachment);
            body = o.body;

            o.attachment = nullptr;

            return *this;
        }

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
            CameraAction camera;
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
