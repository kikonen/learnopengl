#pragma once

#include <type_traits>

namespace event
{
    enum class Type : std::underlying_type_t<std::byte> {
        none = 0,

        node_add,
        node_added,

        node_remove,
        node_removed,

        // NOTE KI final cleanup (WT => RT)
        node_dispose,

        node_select,
        node_activate,

        type_prepare_view,

        // NOTE KI allow camera to vary independent of active node
        camera_activate,
        camera_activate_next,

        scene_loaded,

        script_run,

        viewport_changed,

        console_execute,
        console_complete,

        app_shutdown,
    };
}
