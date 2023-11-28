#pragma once

#include "BaseLoader.h"

class Camera;

namespace loader {
    struct CameraData {
        bool enabled{ false };

        bool isDefault{ false };

        bool orthagonal{ false };
        std::array<float, 4> viewport{ 0.f };

        float fov{ 45.f };

        glm::vec3 front{ 0.f, 0.f, -1.f };
        glm::vec3 up{ 0.f, 1.f, 0.f };

        // pos relative to owning node
        glm::vec3 pos{ 0.f };
        glm::vec3 rotation{ 0.f };
    };

    class CameraLoader : public BaseLoader
    {
    public:
        CameraLoader(
            Context ctx);

        void loadCamera(
            const YAML::Node& node,
            CameraData& data) const;

        std::unique_ptr<Camera> createCamera(
            const CameraData& data);

    };
}
