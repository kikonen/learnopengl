#pragma once

#include <variant>

#include "script/size.h"

#include "api/Command.h"

#include "api/Cancel.h"
#include "api/InvokeLuaFunction.h"
#include "api/Sync.h"
#include "api/Wait.h"

#include "api/NodeCommand.h"
#include "api/MoveNode.h"
#include "api/MoveSplineNode.h"
#include "api/ResumeNode.h"
#include "api/RotateNode.h"
#include "api/ScaleNode.h"
#include "api/StartNode.h"
#include "api/SetTextNode.h"

#include "api/AudioPlay.h"
#include "api/AudioPause.h"
#include "api/AudioStop.h"

#include "api/ParticleEmit.h"
#include "api/ParticleStop.h"

#include "api/AnimationPlay.h"

struct UpdateContext;
namespace script
{
    constexpr size_t COMMAND_BUFFER_SIZE = std::max({
        sizeof(script::Cancel),
        sizeof(script::Sync),
        sizeof(script::Wait),
        sizeof(script::InvokeLuaFunction),
        // Node
        sizeof(script::AudioPause),
        sizeof(script::AudioPlay),
        sizeof(script::AudioStop),
        sizeof(script::AnimationPlay),
        sizeof(script::ParticleEmit),
        sizeof(script::ParticleStop),
        sizeof(script::MoveNode),
        sizeof(script::MoveSplineNode),
        sizeof(script::RotateNode),
        sizeof(script::ScaleNode),
        sizeof(script::ResumeNode),
        sizeof(script::StartNode),
        sizeof(script::SetTextNode),
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
