#pragma once

#include <array>
#include <string>
#include <vector>
#include <variant>
#include <memory>

#include <glm/vec3.hpp>

#include "ki/size.h"

#include "model/NodeState.h"

#include "audio/size.h"
#include "audio/limits.h"

#include "loader/PhysicsData.h"

#include "script/size.h"

#include "size.h"

#include "Type.h"

struct UpdateContext;
class NodeController;

namespace event {
    struct PhysicsAttach {
        loader::BodyData body;
        loader::GeomData geom;
        bool enabled{ false };
        bool update{ false };

        bool isValid() const noexcept {
            return enabled &&
                (body.type != physics::BodyType::none ||
                geom.type != physics::GeomType::none);
        }
    };

    struct NodeAttach {
        NodeState state;
        PhysicsAttach physics;
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

    struct MeshTypeAction {
        ki::type_id target{ 0 };
    };

    struct ControlAction {
        ki::node_id target{ 0 };
        NodeController* controller{ nullptr };
    };

    struct AudioSourceAction {
        ki::node_id target{ 0 };
        audio::source_id id{ 0 };
    };

    struct AudioListenerAction {
        ki::node_id target{ 0 };
    };

    struct CommandAction {
        ki::node_id target{ 0 };

        script::command_id after{ 0 };
        float duration{ 0 };
        bool relative{ true };
        glm::vec3 data{ 0.f };
        glm::vec3 data2{ 0.f };
    };

    struct ScriptAction {
        ki::node_id target{ 0 };
        script::script_id id{ 0 };
    };
}
