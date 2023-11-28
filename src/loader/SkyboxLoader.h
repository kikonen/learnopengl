#pragma once

#include "BaseLoader.h"

#include "SkyboxData.h"

namespace loader {
    class SkyboxLoader : public BaseLoader
    {
    public:
        SkyboxLoader(
            Context ctx);

        void loadSkybox(
            const YAML::Node& node,
            SkyboxData& data);

        void loadSkyboxFaces(
            const YAML::Node& node,
            SkyboxData& data);

        void attachSkybox(
            const uuids::uuid& rootId,
            const SkyboxData& data);
    };
}
