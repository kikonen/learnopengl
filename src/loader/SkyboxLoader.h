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
            const loader::Node& node,
            SkyboxData& data);

        void loadSkyboxFaces(
            const loader::Node& node,
            SkyboxData& data);

        void attachSkybox(
            const ki::node_id rootId,
            const SkyboxData& data);
    };
}
