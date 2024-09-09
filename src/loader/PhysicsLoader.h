#pragma once

#include <vector>

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "physics/Body.h"
#include "physics/Geom.h"

#include "BaseLoader.h"

#include "PhysicsData.h"

namespace loader {
    class PhysicsLoader : public BaseLoader
    {
    public:
        PhysicsLoader(
            Context ctx);

        void loadPhysics(
            const loader::DocNode& node,
            PhysicsData& data) const;

        void loadBody(
            const loader::DocNode& node,
            physics::Body& data) const;

        physics::Body createBody(
            const PhysicsData& data);

        physics::Geom createGeom(
            const PhysicsData& data);
    };
}
