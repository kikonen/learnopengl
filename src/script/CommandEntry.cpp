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
            if (auto* src = dynamic_cast<Cancel*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<Sync*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<Wait*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<InvokeLuaFunction*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<AudioPause*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<AudioPlay*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<AudioStop*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<AnimationPlay*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<MoveNode*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<MoveSplineNode*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<RotateNode*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<ScaleNode*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<ResumeNode*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<StartNode*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }
            else if (auto* src = dynamic_cast<SetTextNode*>(other_cmd)) {
                m_cmd = assign(m_buffer, src);
            }

            // NOTE KI virtual destructor works via calling base class
            if (useDelete)
                m_cmd->~Command();
        }
        else {
            m_cmd = nullptr;
        }
    }

}
