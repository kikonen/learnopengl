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
    struct AudioSourceAttach {
        audio::sound_id soundId{ 0 };
        std::string name;

        bool isAutoPlay{ false };

        float referenceDistance{ audio::REFERENCE_DISTANCE };
        float maxDistance{ audio::MAX_DISTANCE };
        float rolloffFactor{ audio::ROLLOFF_FACTOR };

        float minGain{ audio::MIN_GAIN };
        float maxGain{ audio::MAX_GAIN };

        bool looping{ false };
        float pitch{ 1.f };
        float gain{ 1.f };

        bool isValid() const noexcept {
            return soundId > 0.f;
        }
    };

    struct AudioListenerAttach {
        bool isDefault{ false };
        float gain{ 0.f };

        bool isValid() const noexcept {
            return gain > 0.f;
        }
    };

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
        AudioListenerAttach audioListener;
        std::vector<AudioSourceAttach> audioSources;
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
        audio::source_id id{ 0 };
    };

    struct AudioListenerAction {
        audio::listener_id id{ 0 };
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
