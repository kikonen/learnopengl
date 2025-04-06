#pragma once

#include <type_traits>

namespace event
{
    enum class Type : std::underlying_type_t<std::byte> {
        none = 0,

        node_add,
        node_added,
        node_select,
        node_activate,

        type_prepare_view,

        // NOTE KI allow camera to vary independent of active node
        camera_activate,

        controller_add,

        audio_listener_activate,

        scene_loaded,

        script_type_bind,
        script_node_bind,
        script_run,

        app_shutdown,
    };
}
