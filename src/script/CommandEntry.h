#pragma once

#include <variant>

#include "script/size.h"

#include "command/Command.h"

#include "command/Cancel.h"
#include "command/CancelMultiple.h"
#include "command/InvokeFunction.h"
#include "command/Sync.h"
#include "command/Wait.h"

#include "command/NodeCommand.h"
#include "command/MoveNode.h"
#include "command/MoveSplineNode.h"
#include "command/MovePathNode.h"
#include "command/ResumeNode.h"
#include "command/RotateNode.h"
#include "command/ScaleNode.h"
#include "command/StartNode.h"
#include "command/SelectNode.h"

#include "command/SetTextNode.h"
#include "command/SetVisibleNode.h"

#include "command/AudioPlay.h"
#include "command/AudioPause.h"
#include "command/AudioStop.h"

#include "command/ParticleEmit.h"
#include "command/ParticleStop.h"

#include "command/AnimationPlay.h"

#include "command/RayCast.h"
#include "command/RayCastMultiple.h"
#include "command/FindPath.h"

#include "command/EmitEvent.h"

struct UpdateContext;
namespace script
{
    constexpr size_t COMMAND_BUFFER_SIZE = std::max({
        sizeof(script::Cancel),
        sizeof(script::CancelMultiple),
        sizeof(script::Sync),
        sizeof(script::Wait),
        sizeof(script::InvokeFunction),
        // Node
        sizeof(script::AudioPause),
        sizeof(script::AudioPlay),
        sizeof(script::AudioStop),
        sizeof(script::AnimationPlay),
        sizeof(script::ParticleEmit),
        sizeof(script::ParticleStop),
        sizeof(script::MoveNode),
        sizeof(script::MoveSplineNode),
        sizeof(script::MovePathNode),
        sizeof(script::RotateNode),
        sizeof(script::ScaleNode),
        sizeof(script::ResumeNode),
        sizeof(script::StartNode),
        sizeof(script::SelectNode),
        sizeof(script::SetTextNode),
        sizeof(script::SetVisibleNode),
        sizeof(script::RayCast),
        sizeof(script::RayCastMultiple),
        sizeof(script::FindPath),
        sizeof(script::EmitEvent),
        });

    class CommandHandle;

    struct CommandEntry {
        friend class CommandHandle;

        // NOTE KI clearing m_buffer is waste of CPU cycles here
        // m_cmd ptr controls access
        CommandEntry()
        {}

        CommandEntry(const CommandEntry& o) = delete;

        CommandEntry(CommandEntry&& o) noexcept
            : afterId{ o.afterId },
            m_id{ o.m_id },
            m_alive{ o.m_alive },
            m_ready{ o.m_ready },
            m_next{ o.m_next }
        {
            moveCommand(o.m_cmd, true);
            o.m_cmd = nullptr;
        }

        template<class T>
        CommandEntry(script::command_id afterId, T&& o) noexcept
        {
            T* data = reinterpret_cast<T*>(&m_buffer);
            *data = std::move(o);
            m_cmd = reinterpret_cast<T*>(&m_buffer);
        }

        ~CommandEntry() {
            if (m_cmd) {
                m_cmd->~Command();
                m_cmd = nullptr;
            }
            m_id = 0;
        }

        CommandEntry& operator=(CommandEntry&& o) noexcept
        {
            if (this == &o) return *this;

            afterId = o.afterId;
            m_id = o.m_id;
            m_alive = o.m_alive;
            m_ready = o.m_ready;
            m_next = o.m_next;

            moveCommand(o.m_cmd, true);
            o.m_cmd = nullptr;

            return *this;
        }

        template<typename T>
        void set(T&& o)
        {
            moveCommand(&o, false);
        }

        void moveCommand(Command* other_cmd, bool useDelete);

        script::command_id m_id{ 0 };

        script::command_id afterId{ 0 };

        bool m_alive : 1 { true };
        bool m_ready : 1 { false };
        bool m_active : 1 { false };

        std::vector<script::command_id> m_next;

        Command* m_cmd{ nullptr };
        uint8_t m_buffer[COMMAND_BUFFER_SIZE];
    };
}
