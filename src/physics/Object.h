#pragma once

#include <glm/glm.hpp>
#include <ode/ode.h>

class Node;

namespace physics {
    class PhysicsEngine;

    enum class BodyType {
        none,
        sphere,
        box,
    };

    struct Body {
        BodyType type{ BodyType::none };

        // NOTE KI *SCALED* using scale of node
        // size{0] == radius
        glm::vec3 size{ 1.f };

        float density{ 1.f };

        // initial values for physics
        glm::vec3 linearVel{ 0.f };
        glm::vec3 angularVel{ 0.f };

        // NOTE KI *ROTATED* using rotation of node
        // axis + angle
        //glm::vec4 quat{ 0.f };
        glm::vec3 rotation{ 0.f };
    };

    enum class GeomType {
        none,
        plane,
        sphere,
        box,
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
            Body body,
            Geom geom)
        : m_body(body),
        m_geom(geom)
        {}

        ~Object();

        void create(
            PhysicsEngine* engine,
            Node* node);

        void updateToPhysics(
            PhysicsEngine* engine,
            Node* node);

        void updateFromPhysics(
            PhysicsEngine* engine,
            Node* node);

        Body m_body;
        Geom m_geom;

        dMass m_mass;
        dBodyID m_bodyId{ nullptr };
        dGeomID m_geomId{ nullptr };

        Node* m_node{ nullptr };
    };
}
