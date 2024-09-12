#pragma once

#include "ki/size.h"

#include "audio/size.h"

#include "event/actions.h"

#include "BaseLoader.h"
#include "AudioData.h"

namespace loader {
    class AudioLoader : public BaseLoader
    {
    public:
        AudioLoader(
            Context ctx);

        void loadAudio(
            const loader::DocNode& node,
            AudioData& data) const;

        void loadListener(
            const loader::DocNode& node,
            ListenerData& data) const;

        void loadSources(
            const loader::DocNode& node,
            std::vector<SourceData>& sources) const;

        void loadSource(
            const loader::DocNode& node,
            SourceData& data) const;

        std::vector<event::AudioSourceAttach> createSources(
            const std::vector<SourceData>& sources);

        event::AudioSourceAttach createSource(
            const SourceData& data);

        event::AudioListenerAttach createListener(
            const ListenerData& data);
    };
}
