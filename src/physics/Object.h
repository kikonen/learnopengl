#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <ode/ode.h>

class Node;

namespace physics {
    class PhysicsEngine;

    enum class BodyType {
        none,
        box,
        sphere,
        capsule,
        cylinder,
    };

    struct Body {
        BodyType type{ BodyType::none };

        bool kinematic{ false };

        // NOTE KI *SCALED* using scale of node
        // size{0] == radius
        glm::vec3 size{ 1.f };

        float density{ 1.f };

        // initial values for physics
        glm::vec3 linearVel{ 0.f };
        glm::vec3 angularVel{ 0.f };

        // NOTE KI *ROTATED* using rotation of node
        // axis + angle
        glm::quat quat;

        glm::vec3 rotation{ 0.f };
    };

    enum class GeomType {
        none,
        plane,
        box,
        sphere,
        capsule,
        cylinder,
    };

    struct Geom {
        GeomType type{ GeomType::none };

        // NOTE KI *SCALED* using scale of node
        // size{0] == radius
        glm::vec3 size{ 1.f };

        glm::vec4 plane{ 0.f, 1.f, 0.f, 0.f };

        unsigned int category{ UINT_MAX };
        unsigned int collide{ UINT_MAX };
    };

    struct Object {
    public:
        Object(
            bool update,
            Body body,
            Geom geom)
        : m_update(update),
            m_body(body),
            m_geom(geom)
        {}

        ~Object();

        inline bool ready() const { return m_geomId || m_bodyId; }

        void prepare(
            PhysicsEngine* engine,
            Node* node);

        void updateToPhysics(bool force);
        void updateFromPhysics();

        bool m_update{ false };
        Body m_body;
        Geom m_geom;

        dMass m_mass;
        dBodyID m_bodyId{ nullptr };
        dGeomID m_geomId{ nullptr };

        Node* m_node{ nullptr };
    };
}
