#pragma once

#include "ki/size.h"

#include "audio/size.h"
#include "audio/Source.h"
#include "audio/Listener.h"

#include "BaseLoader.h"
#include "AudioData.h"

struct AudioListenerDefinition;
struct AudioSourceDefinition;

namespace loader {
    class AudioLoader : public BaseLoader
    {
    public:
        AudioLoader(
            std::shared_ptr<Context> ctx);

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

        std::unique_ptr<std::vector<AudioSourceDefinition>> createSourceDefinitions(
            const std::vector<SourceData>& sources);

        void createSourceDefinition(
            const SourceData& data,
            AudioSourceDefinition& source);

        std::unique_ptr<AudioListenerDefinition> createListenerDefinition(
            const ListenerData& data);

        std::unique_ptr<std::vector<audio::Source>> createSources(
            const std::vector<SourceData>& sources);
    };
}
