#pragma once

namespace loader {
    enum class ControllerType : std::underlying_type_t<std::byte> {
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
