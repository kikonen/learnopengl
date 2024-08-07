#pragma once

#include <array>
#include <string>
#include <variant>
#include <memory>

#include <glm/vec3.hpp>

#include "ki/size.h"

#include "audio/size.h"
#include "audio/limits.h"

#include "physics/Geom.h"
#include "physics/Body.h"

#include "script/size.h"

#include "size.h"

#include "Type.h"

struct UpdateContext;
class NodeController;

namespace event {
    struct PhysicsData {
        bool update{ false };
        physics::Body body;
        physics::Geom geom;
    };

    struct AudioSourceData {
        audio::sound_id soundId{ 0 };
        uint8_t index{ 0 };
        bool isAutoPlay{ false };

        float referenceDistance{ audio::REFERENCE_DISTANCE };
        float maxDistance{ audio::MAX_DISTANCE };
        float rolloffFactor{ audio::ROLLOFF_FACTOR };

        float minGain{ audio::MIN_GAIN };
        float maxGain{ audio::MAX_GAIN };

        bool looping{ false };
        float pitch{ 1.f };
        float gain{ 1.f };
    };

    struct AudioListenerData {
        bool isDefault{ false };
        float gain{ 1.f };
    };

    // Blobdata is for doing passing abnormally large event blobs
    // for initalization, which are not needed in normal event logic
    // => To reduce bloating of Event struct size
    struct BlobData {
        BlobData() {};
        ~BlobData();

        union Body {
            Body() {};
            ~Body();

            PhysicsData physics;
            AudioSourceData audioSource;
            AudioListenerData audioListener;
        } body;
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

    struct AudioInitAction {
        ki::node_id target{ 0 };
    };

    struct PhysicsAction {
        ki::node_id target{ 0 };
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

    struct Event {
        Event(Type a_type)
            : type{ a_type },
              body{}
        {}

        Event(Event& o) noexcept
            : type{ o.type },
            blob{ std::move(o.blob) },
            body{ o.body }
        {}

        Event(Event&& o) noexcept
            : type{ o.type },
            blob{ std::move(o.blob) },
            body{ o.body }
        {}

        Type type;

        std::unique_ptr<BlobData> blob;
        union Body {
            NodeAction node;
            MeshTypeAction meshType;
            ControlAction control;
            AudioInitAction audioInit;
            AudioSourceAction audioSource;
            AudioListenerAction audioListener;
            PhysicsAction physics;
            CommandAction command;
            ScriptAction script;
        } body;
    };

    struct EventPolicies
    {
        static Type getEvent(const Event& e) {
            return e.type;
        }
    };
}
