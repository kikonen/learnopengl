#pragma once

#include <memory>

#include "audio/size.h"

namespace audio
{
    struct Listener;
}

class NodeType;

struct AudioListenerDefinition
{
    float m_gain{ 1.f };

    static std::unique_ptr<audio::Listener> createAudioListener(
        const NodeType* type);
};
