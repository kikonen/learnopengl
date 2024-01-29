#pragma once

#include "script/size.h"


namespace script {
    struct CommandEntry;

    class CommandHandle final
    {
        friend struct CommandEntry;

    public:
        CommandHandle()
            : m_handleIndex{ 0 },
            m_id{ 0 }
        {}

        CommandHandle(
            uint32_t handleIndex,
            script::command_id id
        ) : m_handleIndex{ handleIndex },
            m_id{ id }
        {}

        CommandHandle(const CommandHandle& o)
            : m_handleIndex{ o.m_handleIndex },
            m_id{ o.m_id }
        {}

        CommandHandle(CommandHandle&& o) noexcept
            : m_handleIndex{ o.m_handleIndex },
            m_id{ o.m_id }
        {}

        ~CommandHandle() = default;

        CommandHandle& operator=(const CommandHandle& o)
        {
            if (&o == this) return *this;
            m_handleIndex = o.m_handleIndex;
            m_id = o.m_id;
            return *this;
        }

        CommandHandle& operator=(CommandHandle&& o) noexcept
        {
            m_handleIndex = o.m_handleIndex;
            m_id = o.m_id;
            return *this;
        }

        CommandHandle& operator=(const CommandEntry* entry) noexcept;

        bool operator==(const CommandHandle& o) const noexcept
        {
            return m_handleIndex == o.m_handleIndex &&
                m_id == o.m_id;
        }

        bool present() const noexcept { return m_handleIndex > 0; }
        bool isNull() const noexcept { return m_handleIndex == 0; }
        operator int() const { return m_handleIndex; }

        void release() noexcept;

        void reset() noexcept {
            m_handleIndex = 0;
            m_id = 0;
        }

        CommandEntry* toCommand() const noexcept;
        script::command_id toId() const noexcept { return m_id; }

        static CommandHandle allocate(script::command_id id) noexcept;

        static CommandHandle toHandle(script::command_id id) noexcept;

        static CommandEntry* toCommand(script::command_id id) noexcept;

        static void clear() noexcept;

        static script::command_id nextId();

    public:
        static CommandHandle NULL_HANDLE;

    private:
        uint32_t m_handleIndex;
        script::command_id m_id;
    };
}
