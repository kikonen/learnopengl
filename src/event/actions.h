#pragma once

#include <array>
#include <string>
#include <vector>
#include <variant>
#include <memory>

#include <glm/vec3.hpp>

#include "ki/size.h"

#include "model/CreateState.h"

#include "audio/size.h"
#include "audio/limits.h"

#include "script/size.h"

#include "size.h"

#include "Type.h"

namespace pool {
    struct NodeHandle;
}

namespace event {
    struct NodeAttach {
        model::CreateState state;
    };

    // Blobdata is for doing passing abnormally large event blobs
    // for initalization, which are not needed in normal event logic
    // => To reduce bloating of Event struct size
    struct BlobData {
        BlobData() {};
        ~BlobData();

        NodeAttach body;
    };

    struct NodeAction {
        ki::node_id target{ 0 };
        ki::node_id parentId{ 0 };
        ki::socket_id socketId{ 0 };
        int offset{ 0 };
    };

    struct NodeTypeAction {
        ki::type_id target{ 0 };
    };

    struct ScriptAction {
        ki::node_id target{ 0 };
        script::script_id id{ 0 };
        bool global{ false };
    };

    struct SelectAction {
        ki::node_id target{ 0 };
        bool select{ false };
        bool append{ false };
    };

    struct ViewportAction {
        uint8_t layer{ 0 };
        glm::uvec2 aspectRatio{ 1 };
    };
}
