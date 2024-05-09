#pragma once

#include <vector>

#include "ki/size.h"

#include "BaseLoader.h"

#include "PhysicsData.h"

namespace loader {
    class PhysicsLoader : public BaseLoader
    {
    public:
        PhysicsLoader(
            Context ctx);

        void loadPhysics(
            const loader::Node& node,
            PhysicsData& data) const;

        void loadBody(
            const loader::Node& node,
            physics::Body& data) const;

        void loadGeom(
            const loader::Node& node,
            physics::Geom& data) const;

        void createObject(
            const PhysicsData& data,
            const ki::node_id nodeId);
    };
}
