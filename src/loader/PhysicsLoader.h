#pragma once

#include <vector>
#include <memory>

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "physics/Body.h"
#include "physics/Shape.h"

#include "BaseLoader.h"

#include "PhysicsData.h"

struct PhysicsDefinition;

namespace loader {
    class PhysicsLoader : public BaseLoader
    {
    public:
        PhysicsLoader(
            const std::shared_ptr<Context>& ctx);

        void loadPhysics(
            const loader::DocNode& node,
            PhysicsData& data) const;

        void loadBody(
            const loader::DocNode& node,
            loader::BodyData& data) const;

        std::unique_ptr<PhysicsDefinition> createPhysicsDefinition(
            const loader::PhysicsData& data);
    };
}
