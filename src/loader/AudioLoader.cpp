#include "AudioLoader.h"

#include "ki/limits.h"

#include "asset/Assets.h"

#include "util/util.h"
#include "util/file.h"

#include "audio/Source.h"
#include "audio/Listener.h"
#include "audio/AudioSystem.h"

#include "registry/Registry.h"

#include "loader/document.h"
#include "loader_util.h"

namespace loader
{
    AudioLoader::AudioLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void AudioLoader::loadAudio(
        const loader::DocNode& node,
        AudioData& data) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "listener") {
                loadListener(v, data.listener);
            } else if (k == "source") {
                SourceData& source = data.sources.empty() ? data.sources.emplace_back() : data.sources[0];
                loadSource(v, source);
            } else if (k == "sources") {
                // NOTE KI no sensible strategy to merge lists
                data.sources.clear();
                loadSources(v, data.sources);
            } else {
                reportUnknown("audio_entry", k, v);
            }
        }
    }

    void AudioLoader::loadListener(
        const loader::DocNode& node,
        ListenerData& data) const
    {
        data.enabled = true;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "default") {
                data.isDefault = readBool(v);
            }
            else if (k == "gain") {
                data.gain = readFloat(v);
            }
            else {
                reportUnknown("listener_entry", k, v);
            }
        }
    }

    void AudioLoader::loadSources(
        const loader::DocNode& node,
        std::vector<SourceData>& sources) const
    {
        int index = 0;
        for (const auto& entry : node.getNodes()) {
            SourceData& data = sources.emplace_back();
            loadSource(entry, data);
        }
    }

    void AudioLoader::loadSource(
        const loader::DocNode& node,
        SourceData& data) const
    {
        data.enabled = true;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "autoplay") {
                data.isAutoPlay = readBool(v);
            }
            else if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "path") {
                data.path = readString(v);
            }
            else if (k == "reference_distance") {
                data.referenceDistance = readFloat(v);
            }
            else if (k == "max_distance") {
                data.maxDistance = readFloat(v);
            }
            else if (k == "rolloff_factor") {
                data.rolloffFactor = readFloat(v);
            }
            else if (k == "min_gain") {
                data.minGain = readFloat(v);
            }
            else if (k == "max_gain") {
                data.maxGain = readFloat(v);
            }
            else if (k == "gain") {
                data.gain = readFloat(v);
            }
            else if (k == "pitch") {
                data.pitch = readFloat(v);
            }
            else if (k == "looping" || k == "loop") {
                data.looping = readBool(v);
            }
            else {
                reportUnknown("source_entry", k, v);
            }
        }

        if (data.name.empty()) {
            data.name = data.path;
        }
    }

    std::unique_ptr<std::vector<audio::Source>> AudioLoader::createSources(
        const std::vector<SourceData>& sources)
    {
        std::unique_ptr<std::vector<audio::Source>> result =
            std::make_unique<std::vector<audio::Source>>();

        for (const auto& data : sources) {
            auto& src = result->emplace_back();
            createSource(data, src);
            if (!src.m_soundId) {
                result->pop_back();
            }
        }
        return result->empty() ? nullptr : std::move(result);
    }

    void AudioLoader::createSource(
        const SourceData& data,
        audio::Source& source)
    {
        const auto& assets = Assets::get();

        audio::sound_id soundId;
        {
            std::string fullPath = util::joinPath(assets.assetsDir, data.path);
            soundId = audio::AudioSystem::get().registerSound(fullPath);

            if (!soundId) return;
        }

        source.m_id = SID(data.name);
        source.m_soundId = soundId;
        source.m_autoPlay = data.isAutoPlay;

        source.m_referenceDistance = data.referenceDistance;
        source.m_maxDistance = data.maxDistance;
        source.m_rolloffFactor = data.rolloffFactor;

        source.m_minGain = data.minGain;
        source.m_maxGain = data.maxGain;

        source.m_looping = data.looping;

        source.m_pitch = data.pitch;
        source.m_gain = data.gain;
    }

    std::unique_ptr<audio::Listener> AudioLoader::createListener(
        const ListenerData& data)
    {
        if (!data.enabled) return nullptr;

        std::unique_ptr<audio::Listener> listener = std::make_unique<audio::Listener>();

        listener->m_default = data.isDefault;
        listener->m_gain = data.gain;

        return listener;
    }
}
