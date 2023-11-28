#pragma once

#include <vector>

#include "BaseLoader.h"

#include "PhysicsData.h"

namespace loader {
    class PhysicsLoader : public BaseLoader
    {
    public:
        PhysicsLoader(
            Context ctx);

        void loadPhysics(
            const YAML::Node& node,
            PhysicsData& data) const;

        void loadBody(
            const YAML::Node& node,
            BodyData& data) const;

        void loadGeom(
            const YAML::Node& node,
            GeomData& data) const;

        std::unique_ptr<physics::Object> createPhysicsObject(
            const PhysicsData& data,
            const int cloneIndex,
            const glm::uvec3& tile);

    };
}
