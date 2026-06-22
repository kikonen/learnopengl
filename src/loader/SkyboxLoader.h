#pragma once

#include "BaseLoader.h"

#include "SkyboxData.h"

namespace loader {
    class SkyboxLoader : public BaseLoader
    {
    public:
        SkyboxLoader(
            const util::Ref<Context>& ctx);

        void loadSkybox(
            const loader::DocNode& node,
            SkyboxData& data);

        void loadSkyboxFaces(
            const loader::DocNode& node,
            SkyboxData& data);

        void attachSkybox(
            const SkyboxData& data);
    };
}
