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
    class AudioLoader;
    class ControllerLoader;
    class GeneratorLoader;
    class ParticleLoader;
    class PhysicsLoader;
    class ScriptLoader;

    class EntityLoader : public BaseLoader
    {
    public:
        EntityLoader(
            Context ctx);

        void loadEntities(
            const YAML::Node& node,
            std::vector<EntityData>& entities,
            MaterialLoader& materialLoader,
            CustomMaterialLoader& customMaterialLoader,
            SpriteLoader& spriteLoader,
            CameraLoader& cameraLoader,
            LightLoader& lightLoader,
            AudioLoader& audioLoader,
            ControllerLoader& controllerLoader,
            GeneratorLoader& generatorLoader,
            ParticleLoader& particleLoader,
            PhysicsLoader& physicsLoader,
            ScriptLoader& scriptLoader) const;

        void loadEntity(
            const YAML::Node& node,
            EntityData& data,
            MaterialLoader& materialLoader,
            CustomMaterialLoader& customMaterialLoader,
            SpriteLoader& spriteLoader,
            CameraLoader& cameraLoader,
            LightLoader& lightLoader,
            AudioLoader& audioLoader,
            ControllerLoader& controllerLoader,
            GeneratorLoader& generatorLoader,
            ParticleLoader& particleLoader,
            PhysicsLoader& physicsLoader,
            ScriptLoader& scriptLoader) const;

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
            AudioLoader& audioLoader,
            ControllerLoader& controllerLoader,
            GeneratorLoader& generatorLoader,
            ParticleLoader& particleLoader,
            PhysicsLoader& physicsLoader,
            ScriptLoader& scriptLoader) const;

        void loadText(
            const YAML::Node& node,
            TextData& data) const;

        void loadLods(
            const YAML::Node& node,
            std::vector<LodData>& lods,
            MaterialLoader& materialLoader) const;

        void loadLod(
            const YAML::Node& node,
            LodData& data,
            MaterialLoader& materialLoader) const;

        void loadAnimations(
            const YAML::Node& node,
            std::vector<AnimationData>& animations) const;

        void loadAnimation(
            const YAML::Node& node,
            AnimationData& data) const;
    };
}
