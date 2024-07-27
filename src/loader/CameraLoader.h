#pragma once

#include "BaseLoader.h"
#include "CameraData.h"

class CameraComponent;

namespace loader {
    class CameraLoader : public BaseLoader
    {
    public:
        CameraLoader(
            Context ctx);

        void loadCamera(
            const loader::DocNode& node,
            CameraData& data) const;

        std::unique_ptr<CameraComponent> createCamera(
            const CameraData& data);

    };
}
