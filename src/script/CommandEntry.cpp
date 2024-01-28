#include "CommandEntry.h"

namespace {
    template<typename T>
    T* assign(uint8_t* dst, T* src)
    {
        new(dst) T(std::move(*src));
        return reinterpret_cast<T*>(dst);
    }
}

namespace script {
    void CommandEntry::moveCommand(Command* other_cmd, bool useDelete) {
        if (other_cmd) {
            if (auto* src = dynamic_cast<CancelCommand*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
                if (useDelete) src->~CancelCommand();
            }
            else if (auto* src = dynamic_cast<Sync*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
                if (useDelete) src->~Sync();
            }
            else if (auto* src = dynamic_cast<Wait*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
                if (useDelete) src->~Wait();
            }
            else if (auto* src = dynamic_cast<InvokeLuaFunction*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
                if (useDelete) src->~InvokeLuaFunction();
            }
            else if (auto* src = dynamic_cast<AudioPause*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
                if (useDelete) src->~AudioPause();
            }
            else if (auto* src = dynamic_cast<AudioPlay*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
                if (useDelete) src->~AudioPlay();
            }
            else if (auto* src = dynamic_cast<AudioStop*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
                if (useDelete) src->~AudioStop();
            }
            else if (auto* src = dynamic_cast<MoveNode*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
                if (useDelete) src->~MoveNode();
            }
            else if (auto* src = dynamic_cast<MoveSplineNode*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
                if (useDelete) src->~MoveSplineNode();
            }
            else if (auto* src = dynamic_cast<RotateNode*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
                if (useDelete) src->~RotateNode();
            }
            else if (auto* src = dynamic_cast<ScaleNode*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
                if (useDelete) src->~ScaleNode();
            }
            else if (auto* src = dynamic_cast<ResumeNode*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
                if (useDelete) src->~ResumeNode();
            }
            else if (auto* src = dynamic_cast<StartNode*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
                if (useDelete) src->~StartNode();
            }
        }
        else {
            m_cmd = nullptr;
            memset(m_buffer, 0, sizeof(m_buffer));
        }
    }

}
