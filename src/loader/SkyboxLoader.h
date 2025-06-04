#pragma once

#include "BaseLoader.h"

#include "SkyboxData.h"

namespace loader {
    class SkyboxLoader : public BaseLoader
    {
    public:
        SkyboxLoader(
            std::shared_ptr<Context> ctx);

        void loadSkybox(
            const loader::DocNode& node,
            SkyboxData& data);

        void loadSkyboxFaces(
            const loader::DocNode& node,
            SkyboxData& data);

        void attachSkybox(
            const ki::node_id rootId,
            const SkyboxData& data);
    };
}
