#pragma once

#include <vector>

#include "BaseLoader.h"
#include "EntityData.h"

namespace loader {
    class MaterialLoader;
    class CustomMaterialLoader;
    class SpriteLoader;
    class CameraLoader;
    class LightLoader;
    class ControllerLoader;
    class GeneratorLoader;
    class PhysicsLoader;

    class EntityLoader : public BaseLoader
    {
    public:
        EntityLoader(
            Context ctx);

        void loadEntities(
            const YAML::Node& doc,
            std::vector<EntityData>& entities,
            MaterialLoader& materialLoader,
            CustomMaterialLoader& customMaterialLoader,
            SpriteLoader& spriteLoader,
            CameraLoader& cameraLoader,
            LightLoader& lightLoader,
            ControllerLoader& controllerLoader,
            GeneratorLoader& generatorLoader,
            PhysicsLoader& physicsLoader);

        void loadEntity(
            const YAML::Node& node,
            EntityData& data,
            MaterialLoader& materialLoader,
            CustomMaterialLoader& customMaterialLoader,
            SpriteLoader& spriteLoader,
            CameraLoader& cameraLoader,
            LightLoader& lightLoader,
            ControllerLoader& controllerLoader,
            GeneratorLoader& generatorLoader,
            PhysicsLoader& physicsLoader) const;

        void loadEntityClone(
            const YAML::Node& node,
            EntityCloneData& data,
            std::vector<EntityCloneData>& clones,
            bool recurse,
            MaterialLoader& materialLoader,
            CustomMaterialLoader& customMaterialLoader,
            SpriteLoader& spriteLoader,
            CameraLoader& cameraLoader,
            LightLoader& lightLoader,
            ControllerLoader& controllerLoader,
            GeneratorLoader& generatorLoader,
            PhysicsLoader& physicsLoader) const;
    };
}
