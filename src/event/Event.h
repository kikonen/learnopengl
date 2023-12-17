#pragma once

#include <array>
#include <string>
#include <variant>
#include <memory>

#include <glm/vec3.hpp>

#include "ki/size.h"
#include "ki/uuid.h"

#include "audio/size.h"
#include "script/size.h"

class UpdateContext;
class Node;
class NodeController;

namespace event {
    enum class Type {
        none = 0,

        node_add,
        node_added,
        node_change_parent,
        node_select,
        node_activate,

        // NOTE KI allow camera to vary independent of active node
        camera_activate,

        controller_add,

        audio_listener_add,
        audio_source_add,
        audio_listener_update,
        audio_source_update,
        audio_listener_activate,

        audio_source_play,
        audio_source_stop,
        audio_source_pause,

        animate_wait,
        animate_move,
        animate_rotate,

        scene_loaded,

        script_bind,
        script_run,
    };

    struct NodeAction {
        Node* target{ nullptr };
        uuids::uuid parentId;
    };

    struct ControlAction {
		ki::object_id target{ 0 };
        NodeController* controller{ nullptr };
    };

    struct NodeAudioSourceAction {
        ki::object_id target{ 0 };

        audio::sound_id soundId{ 0 };
        int index{ 0 };
        bool isAutoPlay{ false };
        bool looping{ false };
        float pitch{ 1.f };
        float gain{ 1.f };
    };

    struct NodeAudioListenerAction {
        ki::object_id target{ 0 };

        bool isDefault{ false };
        float gain{ 1.f };
    };

    struct AudioSourceAction {
        audio::source_id id{ 0 };
    };

    struct AudioListenerAction {
        audio::listener_id id{ 0 };
    };

    struct AnimateAction {
		ki::object_id target{ 0 };

        int after{ 0 };
        float duration{ 0 };
        bool relative{ true };
        glm::vec3 data{ 0.f };
        glm::vec3 data2{ 0.f };
    };

    struct ScriptAction {
        ki::object_id target{ 0 };
        script::script_id id{ 0 };
    };

    struct Event {
        Type type;
        ki::event_id id;

        union {
            NodeAction node;
            ControlAction control;
            NodeAudioSourceAction nodeAudioSource;
            NodeAudioListenerAction nodeAudioListener;
            AudioSourceAction audioSource;
            AudioListenerAction audioListener;
            AnimateAction animate;
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
