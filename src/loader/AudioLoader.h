#pragma once

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "audio/size.h"

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

        void createAudio(
            const AudioData& data,
            const pool::NodeHandle handle);

        void createSources(
            const std::vector<SourceData>& sources,
            const pool::NodeHandle handle);

        void createSource(
            const SourceData& data,
            const pool::NodeHandle handle,
            const uint8_t index);

        void createListener(
            const ListenerData& data,
            const pool::NodeHandle handle);
    };
}
