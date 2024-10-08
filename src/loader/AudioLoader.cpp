#include "AudioLoader.h"

#include "ki/limits.h"

#include "asset/Assets.h"

#include "util/Util.h"

#include "audio/Source.h"
#include "audio/Listener.h"
#include "audio/AudioEngine.h"

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

    std::vector<event::AudioSourceAttach> AudioLoader::createSources(
        const std::vector<SourceData>& sources)
    {
        std::vector<event::AudioSourceAttach> result;
        uint8_t index = 0;
        for (const auto& data : sources) {
            const auto& src = createSource(data);
            if (src.soundId) {
                result.push_back(src);
            }
        }
        return result;
    }

    event::AudioSourceAttach AudioLoader::createSource(
        const SourceData& data)
    {
        const auto& assets = Assets::get();

        std::string fullPath = util::joinPath(assets.assetsDir, data.path);
        auto soundId = audio::AudioEngine::get().registerSound(fullPath);

        if (!soundId) return {};

        return {
            .soundId = soundId,
            .name = data.name,
            .isAutoPlay = data.isAutoPlay,
            .referenceDistance = data.referenceDistance,
            .maxDistance = data.maxDistance,
            .rolloffFactor = data.rolloffFactor,
            .minGain = data.minGain,
            .maxGain = data.maxGain,
            .looping = data.looping,
            .pitch = data.pitch,
            .gain = data.gain,
        };
    }

    event::AudioListenerAttach AudioLoader::createListener(
        const ListenerData& data)
    {
        if (!data.enabled) return {};

         return {
            .isDefault = data.isDefault,
            .gain = data.gain,
        };
    }
}
