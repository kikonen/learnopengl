#include "AudioLoader.h"

#include "ki/yaml.h"
#include "ki/limits.h"

#include "util/Util.h"

#include "audio/Source.h"
#include "audio/Listener.h"
#include "audio/AudioEngine.h"

#include "event/Dispatcher.h"
#include "registry/Registry.h"


namespace loader
{
    AudioLoader::AudioLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void AudioLoader::loadAudio(
        const YAML::Node& node,
        AudioData& data) const
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

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
        const YAML::Node& node,
        ListenerData& data) const
    {
        data.enabled = true;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

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
        const YAML::Node& node,
        std::vector<SourceData>& sources) const
    {
        int index = 0;
        for (const auto& entry : node) {
            SourceData& data = sources.emplace_back();
            loadSource(entry, data);
        }
    }

    void AudioLoader::loadSource(
        const YAML::Node& node,
        SourceData& data) const
    {
        data.enabled = true;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "autoplay") {
                data.isAutoPlay = readBool(v);
            }
            else if (k == "path") {
                data.path = readString(v);
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
    }

    void AudioLoader::createAudio(
        const AudioData& data,
        ki::node_id nodeId)
    {
        createListener(data.listener, nodeId);
        createSources(data.sources, nodeId);
    }

    void AudioLoader::createSources(
        const std::vector<SourceData>& sources,
        const ki::node_id nodeId)
    {
        int index = 0;
        for (const auto& data : sources) {
            createSource(data, nodeId, index++);
        }
    }

    void AudioLoader::createSource(
        const SourceData& data,
        const ki::node_id nodeId,
        const int index)
    {
        if (!data.enabled) return;

        std::string fullPath = util::joinPath(m_assets.assetsDir, data.path);
        auto soundId = m_registry->m_audioEngine->registerSound(fullPath);

        if (!soundId) return;

        {
            event::Event evt { event::Type::audio_source_add };
            auto& body = evt.body.nodeAudioSource = {
                .target = nodeId,
                .soundId = soundId,
                .index = index,
                .isAutoPlay = data.isAutoPlay,
                .looping = data.looping,
                .pitch = data.pitch,
                .gain = data.gain,
            };
            m_dispatcher->send(evt);
        }
    }

    void AudioLoader::createListener(
        const ListenerData& data,
        const ki::node_id nodeId)
    {
        if (!data.enabled) return;

        {
            event::Event evt { event::Type::audio_listener_add };
            auto& body = evt.body.nodeAudioListener = {
                .target = nodeId,
                .isDefault = data.isDefault,
                .gain = data.gain,
            };
            m_dispatcher->send(evt);
        }
    }
}
