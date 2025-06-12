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

namespace event {
    struct NodeAttach {
        CreateState state;
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
    };

    struct NodeTypeAction {
        ki::type_id target{ 0 };
    };

    struct AudioSourceAction {
        ki::node_id target{ 0 };
        audio::source_id id{ 0 };
    };

    struct AudioListenerAction {
        ki::node_id target{ 0 };
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
}
