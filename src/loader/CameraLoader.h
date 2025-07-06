#pragma once

#include "BaseLoader.h"
#include "CameraData.h"

struct CameraComponentDefinition;

namespace loader {
    class CameraLoader : public BaseLoader
    {
    public:
        CameraLoader(
            std::shared_ptr<Context> ctx);

        void loadCamera(
            const loader::DocNode& node,
            CameraData& data) const;

        void loadPath(
            const loader::DocNode& node,
            std::vector<glm::vec3>& path) const;

        std::unique_ptr<CameraComponentDefinition> createDefinition(
            const CameraData& data);

    };
}
