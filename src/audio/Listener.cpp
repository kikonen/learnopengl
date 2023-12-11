#include "Listener.h"

#include <mutex>


namespace {
    audio::listener_id idBase{ 0 };

    std::mutex id_lock{};

    audio::listener_id nextID()
    {
        std::lock_guard<std::mutex> lock(id_lock);
        return ++idBase;
    }
}

namespace audio
{
    Listener::Listener()
        : m_id(nextID())
    {}

    Listener::~Listener()
    {}
}
