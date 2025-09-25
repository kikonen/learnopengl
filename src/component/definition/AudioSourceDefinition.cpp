#include "AudioSourceDefinition.h"

#include "asset/Assets.h"

#include "model/NodeType.h"

#include "audio/Source.h"

std::unique_ptr<std::vector<audio::Source>> AudioSourceDefinition::createAudioSources(
    const model::NodeType* type)
{
    if (!type->m_audioSourceDefinitions) return nullptr;

    const auto& sources = *type->m_audioSourceDefinitions;

    auto result = std::make_unique<std::vector<audio::Source>>();

    for (const auto& data : sources) {
        auto& src = result->emplace_back();
        createAudioSource(data, src);
        if (!src.m_soundId) {
            result->pop_back();
        }
    }
    return result->empty() ? nullptr : std::move(result);
}

void AudioSourceDefinition::createAudioSource(
    const AudioSourceDefinition& data,
    audio::Source& source)
{
    const auto& assets = Assets::get();

    source.m_id = data.m_sourceId;
    source.m_soundId = data.m_soundId;

    source.m_referenceDistance = data.m_referenceDistance;
    source.m_maxDistance = data.m_maxDistance;
    source.m_rolloffFactor = data.m_rolloffFactor;

    source.m_minGain = data.m_minGain;
    source.m_maxGain = data.m_maxGain;

    source.m_looping = data.m_looping;

    source.m_pitch = data.m_pitch;
    source.m_gain = data.m_gain;
}
