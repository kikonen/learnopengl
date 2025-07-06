#include "AudioListenerDefinition.h"

#include "model/NodeType.h"

#include "audio/Listener.h"

std::unique_ptr<audio::Listener> AudioListenerDefinition::createAudioListener(
    const NodeType* type)
{
    if (!type->m_audioListenerDefinition) return nullptr;

    const auto& data = *type->m_audioListenerDefinition;

    std::unique_ptr<audio::Listener> listener = std::make_unique<audio::Listener>();

    listener->m_gain = data.m_gain;

    return listener;
}
