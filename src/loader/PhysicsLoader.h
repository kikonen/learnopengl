#pragma once

#include <vector>

#include "BaseLoader.h"

namespace physics {
    class Object;
}


namespace loader {
    enum class BodyType {
        none,
        sphere,
        box,
    };

    struct BodyData {
        BodyType type{ BodyType::none };

        // NOTE KI *SCALED* using scale of node
        // size{0] == radius
        glm::vec3 size{ 1.f };

        float mass{ 1.f };

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

    struct GeomData {
        GeomType type{ GeomType::none };

        // NOTE KI *SCALED* using scale of node
        // size{0] == radius
        glm::vec3 size{ 1.f };

        glm::vec4 plane{ 0.f, 1.f, 0.f, 0.f };

        unsigned int category{ UINT_MAX };
        unsigned int collide{ UINT_MAX };
    };

    struct PhysicsData {
        bool enabled{ false };

        std::string space{ "default" };

        BodyData body;
        GeomData geom;
    };

    class PhysicsLoader : public BaseLoader
    {
    public:
        PhysicsLoader(
            Context ctx);

        void loadPhysics(
            const YAML::Node& node,
            PhysicsData& data);

        void loadBody(
            const YAML::Node& node,
            BodyData& data);

        void loadGeom(
            const YAML::Node& node,
            GeomData& data);

        std::unique_ptr<physics::Object> createPhysicsObject(
            const PhysicsData& data,
            const int cloneIndex,
            const glm::uvec3& tile);

    };
}
