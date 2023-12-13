#pragma once

#include "ki/size.h"

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
            const YAML::Node& node,
            AudioData& data) const;

        void loadListener(
            const YAML::Node& node,
            ListenerData& data) const;

        void loadSource(
            const YAML::Node& node,
            SourceData& data) const;

        void createAudio(
            const AudioData& data,
            ki::object_id nodeId);

        void createSource(
            const SourceData& data,
            ki::object_id nodeId);

        void createListener(
            const ListenerData& data,
            ki::object_id nodeId);
    };
}
