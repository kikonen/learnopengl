#pragma once

namespace loader {
    enum class ControllerType {
        none,
        pawn,
        camera_zoom,
    };

    struct ControllerData {
        bool enabled{ false };
        ControllerType type{ ControllerType::none };

        int mode{ 0 };
        float speed{ 0.f };
    };
}