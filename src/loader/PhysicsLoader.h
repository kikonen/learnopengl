#pragma once

#include <vector>

#include "ki/size.h"

#include "pool/NodeHandle.h"

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

        void loadGeom(
            const loader::DocNode& node,
            physics::Geom& data) const;

        void loadMask(
            const loader::DocNode& node,
            uint32_t& mask) const;

        void createObject(
            const PhysicsData& data,
            const pool::NodeHandle handle);
    };
}
