#pragma once

#include "BaseLoader.h"
#include "CameraData.h"

class Camera;

namespace loader {
    class CameraLoader : public BaseLoader
    {
    public:
        CameraLoader(
            Context ctx);

        void loadCamera(
            const loader::DocNode& node,
            CameraData& data) const;

        std::unique_ptr<Camera> createCamera(
            const CameraData& data);

    };
}
