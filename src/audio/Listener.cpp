#include "Listener.h"

#include <fmt/format.h>

#include "util/Log.h"

#include "model/Snapshot.h"

namespace {
}

namespace audio
{
    Listener::Listener(Listener&& o) noexcept
        : m_id{ o.m_id },
        m_default( o.m_default ),
        m_gain{ o.m_gain },
        m_pos{ o.m_pos },
        m_vel{ o.m_vel },
        m_front{ o.m_front },
        m_up{ o.m_up },
        m_matrixLevel{ o.m_matrixLevel },
        m_nodeHandle{ o.m_nodeHandle }
    {}

    Listener::~Listener()
    {}

    Listener& Listener::operator=(Listener&& o) noexcept
    {
        if (&o == this) return *this;

        m_id = o.m_id;
        m_default = o.m_default;
        m_gain = o.m_gain;
        m_pos = o.m_pos;
        m_vel = o.m_vel;
        m_front = o.m_front;
        m_up = o.m_up;
        m_matrixLevel = o.m_matrixLevel;
        m_nodeHandle = o.m_nodeHandle;

        return *this;
    }

    void Listener::prepare()
    {
        // NOTE KI no auto activate
        KI_INFO_OUT(fmt::format("LISTENER: id={}", m_id));
    }

    void Listener::updateFromSnapshot(const Snapshot& snapshot)
    {
        const auto level = snapshot.m_matrixLevel;
        if (m_matrixLevel == level) return;
        m_matrixLevel = level;

        m_pos = snapshot.getWorldPosition();
        m_front = glm::normalize(snapshot.getViewFront());
        m_up = glm::normalize(snapshot.getViewUp());

        updatePos();
    }

    void Listener::update()
    {
        alListenerf(AL_GAIN, m_gain);

        updatePos();
    }

    void Listener::updatePos()
    {
        alListener3f(AL_POSITION, m_pos.x, m_pos.y, m_pos.z);
        alListener3f(AL_VELOCITY, m_vel.x, m_vel.y, m_vel.z);

        float orientation[6] {
            m_front.x, m_front.y, m_front.z,
            m_up.x, m_up.y, m_up.z
        };
        alListenerfv(AL_ORIENTATION, orientation);
    }
}
