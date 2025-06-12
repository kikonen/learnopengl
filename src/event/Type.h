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

        scene_loaded,

        script_run,

        console_execute,
        console_complete,

        app_shutdown,
    };
}
